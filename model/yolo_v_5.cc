//
// Created by Homin Su on 2021/9/3.
//

#include <cmath>

#include "yolo_v_5.h"
#include "../utils.h"
#include "../cuda_utils.h"
#include "../common.hpp"

YoloV5::YoloV5() {
  cudaSetDevice(DEVICE);
}

YoloV5::~YoloV5() {
  if (nullptr != trtModelStream_) {
    delete[] trtModelStream_;
    trtModelStream_ = nullptr;
  }

  if (nullptr != data_) {
    delete[] data_;
    data_ = nullptr;
  }

  if (nullptr != prob_) {
    delete[] prob_;
    prob_ = nullptr;
  }

  // Release stream and buffers
  cudaStreamDestroy(stream_);
  CUDA_CHECK(cudaFree(gpu_buffers_[input_index_]));
  CUDA_CHECK(cudaFree(gpu_buffers_[output_index_]));

  // Destroy the engine
  context_->destroy();
  engine_->destroy();
  runtime_->destroy();
}

bool YoloV5::ParseArgs(int argc, char **argv) {
  if (argc < 3) return false;
  if (std::string(argv[1]) == "-s" && (argc == 5 || argc == 7)) {
    wts_name_ = std::string(argv[2]);
    engine_name_ = std::string(argv[3]);
    auto net = std::string(argv[4]);
    if (net[0] == 's') {
      gd_ = 0.33;
      gw_ = 0.50;
    } else if (net[0] == 'm') {
      gd_ = 0.67;
      gw_ = 0.75;
    } else if (net[0] == 'l') {
      gd_ = 1.0;
      gw_ = 1.0;
    } else if (net[0] == 'x') {
      gd_ = 1.33;
      gw_ = 1.25;
    } else if (net[0] == 'c' && argc == 7) {
      gd_ = atof(argv[5]);
      gw_ = atof(argv[6]);
    } else {
      return false;
    }
    if (net.size() == 2 && net[1] == '6') {
      is_p6_ = true;
    }
  } else if (std::string(argv[1]) == "-d" && argc == 3) {
    engine_name_ = std::string(argv[2]);
  } else {
    return false;
  }
  return true;
}

int YoloV5::get_width(int x, float gw, int divisor) {
  //return math.ceil(x / divisor) * divisor
  if (int(x * gw) % divisor == 0) {
    return int(x * gw);
  }
  return (int(x * gw / divisor) + 1) * divisor;
}

int YoloV5::get_depth(int x, float gd) {
  if (x == 1) {
    return 1;
  } else {
    return round(x * gd) > 1 ? round(x * gd) : 1;
  }
}

ICudaEngine *YoloV5::build_engine(unsigned int maxBatchSize,
                                  IBuilder *builder,
                                  IBuilderConfig *config,
                                  DataType dt,
                                  float &gd,
                                  float &gw,
                                  std::string &wts_name) {
  INetworkDefinition *network = builder->createNetworkV2(0U);

  // Create input tensor of shape {3, INPUT_H, INPUT_W} with name INPUT_BLOB_NAME
  ITensor *data = network->addInput(INPUT_BLOB_NAME, dt, Dims3{3, INPUT_H, INPUT_W});
  assert(data);

  std::map<std::string, Weights> weightMap = loadWeights(wts_name);

  /* ------ yolov5 backbone------ */
  auto focus0 = focus(network, weightMap, *data, 3, get_width(64, gw), 3, "model.0");
  auto conv1 = convBlock(network, weightMap, *focus0->getOutput(0), get_width(128, gw), 3, 2, 1, "model.1");
  auto bottleneck_CSP2 = C3(network,
                            weightMap,
                            *conv1->getOutput(0),
                            get_width(128, gw),
                            get_width(128, gw),
                            get_depth(3, gd),
                            true,
                            1,
                            0.5,
                            "model.2");
  auto conv3 = convBlock(network, weightMap, *bottleneck_CSP2->getOutput(0), get_width(256, gw), 3, 2, 1, "model.3");
  auto bottleneck_csp4 = C3(network,
                            weightMap,
                            *conv3->getOutput(0),
                            get_width(256, gw),
                            get_width(256, gw),
                            get_depth(9, gd),
                            true,
                            1,
                            0.5,
                            "model.4");
  auto conv5 = convBlock(network, weightMap, *bottleneck_csp4->getOutput(0), get_width(512, gw), 3, 2, 1, "model.5");
  auto bottleneck_csp6 = C3(network,
                            weightMap,
                            *conv5->getOutput(0),
                            get_width(512, gw),
                            get_width(512, gw),
                            get_depth(9, gd),
                            true,
                            1,
                            0.5,
                            "model.6");
  auto conv7 = convBlock(network, weightMap, *bottleneck_csp6->getOutput(0), get_width(1024, gw), 3, 2, 1, "model.7");
  auto spp8 =
      SPP(network, weightMap, *conv7->getOutput(0), get_width(1024, gw), get_width(1024, gw), 5, 9, 13, "model.8");

  /* ------ yolov5 head ------ */
  auto bottleneck_csp9 = C3(network,
                            weightMap,
                            *spp8->getOutput(0),
                            get_width(1024, gw),
                            get_width(1024, gw),
                            get_depth(3, gd),
                            false,
                            1,
                            0.5,
                            "model.9");
  auto conv10 = convBlock(network, weightMap, *bottleneck_csp9->getOutput(0), get_width(512, gw), 1, 1, 1, "model.10");

  auto upsample11 = network->addResize(*conv10->getOutput(0));
  assert(upsample11);
  upsample11->setResizeMode(ResizeMode::kNEAREST);
  upsample11->setOutputDimensions(bottleneck_csp6->getOutput(0)->getDimensions());

  ITensor *inputTensors12[] = {upsample11->getOutput(0), bottleneck_csp6->getOutput(0)};
  auto cat12 = network->addConcatenation(inputTensors12, 2);
  auto bottleneck_csp13 = C3(network,
                             weightMap,
                             *cat12->getOutput(0),
                             get_width(1024, gw),
                             get_width(512, gw),
                             get_depth(3, gd),
                             false,
                             1,
                             0.5,
                             "model.13");
  auto conv14 = convBlock(network, weightMap, *bottleneck_csp13->getOutput(0), get_width(256, gw), 1, 1, 1, "model.14");

  auto upsample15 = network->addResize(*conv14->getOutput(0));
  assert(upsample15);
  upsample15->setResizeMode(ResizeMode::kNEAREST);
  upsample15->setOutputDimensions(bottleneck_csp4->getOutput(0)->getDimensions());

  ITensor *inputTensors16[] = {upsample15->getOutput(0), bottleneck_csp4->getOutput(0)};
  auto cat16 = network->addConcatenation(inputTensors16, 2);

  auto bottleneck_csp17 = C3(network,
                             weightMap,
                             *cat16->getOutput(0),
                             get_width(512, gw),
                             get_width(256, gw),
                             get_depth(3, gd),
                             false,
                             1,
                             0.5,
                             "model.17");

  /* ------ detect ------ */
  IConvolutionLayer *det0 = network->addConvolutionNd(*bottleneck_csp17->getOutput(0),
                                                      3 * (Yolo::CLASS_NUM + 5),
                                                      DimsHW{1, 1},
                                                      weightMap["model.24.m.0.weight"],
                                                      weightMap["model.24.m.0.bias"]);
  auto conv18 = convBlock(network, weightMap, *bottleneck_csp17->getOutput(0), get_width(256, gw), 3, 2, 1, "model.18");
  ITensor *inputTensors19[] = {conv18->getOutput(0), conv14->getOutput(0)};
  auto cat19 = network->addConcatenation(inputTensors19, 2);
  auto bottleneck_csp20 = C3(network,
                             weightMap,
                             *cat19->getOutput(0),
                             get_width(512, gw),
                             get_width(512, gw),
                             get_depth(3, gd),
                             false,
                             1,
                             0.5,
                             "model.20");
  IConvolutionLayer *det1 = network->addConvolutionNd(*bottleneck_csp20->getOutput(0),
                                                      3 * (Yolo::CLASS_NUM + 5),
                                                      DimsHW{1, 1},
                                                      weightMap["model.24.m.1.weight"],
                                                      weightMap["model.24.m.1.bias"]);
  auto conv21 = convBlock(network, weightMap, *bottleneck_csp20->getOutput(0), get_width(512, gw), 3, 2, 1, "model.21");
  ITensor *inputTensors22[] = {conv21->getOutput(0), conv10->getOutput(0)};
  auto cat22 = network->addConcatenation(inputTensors22, 2);
  auto bottleneck_csp23 = C3(network,
                             weightMap,
                             *cat22->getOutput(0),
                             get_width(1024, gw),
                             get_width(1024, gw),
                             get_depth(3, gd),
                             false,
                             1,
                             0.5,
                             "model.23");
  IConvolutionLayer *det2 = network->addConvolutionNd(*bottleneck_csp23->getOutput(0),
                                                      3 * (Yolo::CLASS_NUM + 5),
                                                      DimsHW{1, 1},
                                                      weightMap["model.24.m.2.weight"],
                                                      weightMap["model.24.m.2.bias"]);

  auto yolo = addYoLoLayer(network, weightMap, "model.24", std::vector<IConvolutionLayer *>{det0, det1, det2});
  yolo->getOutput(0)->setName(OUTPUT_BLOB_NAME);
  network->markOutput(*yolo->getOutput(0));

  // Build engine
  builder->setMaxBatchSize(maxBatchSize);
  config->setMaxWorkspaceSize(16 * (1 << 20));  // 16MB
#if defined(USE_FP16)
  config->setFlag(BuilderFlag::kFP16);
#elif defined(USE_INT8)
  std::cout << "Your platform support int8: " << (builder->platformHasFastInt8() ? "true" : "false") << std::endl;
  assert(builder->platformHasFastInt8());
  config->setFlag(BuilderFlag::kINT8);
  Int8EntropyCalibrator2 *calibrator =
      new Int8EntropyCalibrator2(1, INPUT_W, INPUT_H, "./coco_calib/", "int8calib.table", INPUT_BLOB_NAME);
  config->setInt8Calibrator(calibrator);
#endif

  std::cout << "Building engine, please wait for a while..." << std::endl;
  ICudaEngine *engine = builder->buildEngineWithConfig(*network, *config);
  std::cout << "Build engine successfully!" << std::endl;

  // Don't need the network any more
  network->destroy();

  // Release host memory
  for (auto &mem: weightMap) {
    free((void *) (mem.second.values));
  }

  return engine;
}

ICudaEngine *YoloV5::build_engine_p6(unsigned int maxBatchSize,
                                     IBuilder *builder,
                                     IBuilderConfig *config,
                                     DataType dt,
                                     float &gd,
                                     float &gw,
                                     std::string &wts_name) {
  INetworkDefinition *network = builder->createNetworkV2(0U);

  // Create input tensor of shape {3, INPUT_H, INPUT_W} with name INPUT_BLOB_NAME
  ITensor *data = network->addInput(INPUT_BLOB_NAME, dt, Dims3{3, INPUT_H, INPUT_W});
  assert(data);

  std::map<std::string, Weights> weightMap = loadWeights(wts_name);

  /* ------ yolov5 backbone------ */
  auto focus0 = focus(network, weightMap, *data, 3, get_width(64, gw), 3, "model.0");
  auto conv1 = convBlock(network, weightMap, *focus0->getOutput(0), get_width(128, gw), 3, 2, 1, "model.1");
  auto c3_2 = C3(network,
                 weightMap,
                 *conv1->getOutput(0),
                 get_width(128, gw),
                 get_width(128, gw),
                 get_depth(3, gd),
                 true,
                 1,
                 0.5,
                 "model.2");
  auto conv3 = convBlock(network, weightMap, *c3_2->getOutput(0), get_width(256, gw), 3, 2, 1, "model.3");
  auto c3_4 = C3(network,
                 weightMap,
                 *conv3->getOutput(0),
                 get_width(256, gw),
                 get_width(256, gw),
                 get_depth(9, gd),
                 true,
                 1,
                 0.5,
                 "model.4");
  auto conv5 = convBlock(network, weightMap, *c3_4->getOutput(0), get_width(512, gw), 3, 2, 1, "model.5");
  auto c3_6 = C3(network,
                 weightMap,
                 *conv5->getOutput(0),
                 get_width(512, gw),
                 get_width(512, gw),
                 get_depth(9, gd),
                 true,
                 1,
                 0.5,
                 "model.6");
  auto conv7 = convBlock(network, weightMap, *c3_6->getOutput(0), get_width(768, gw), 3, 2, 1, "model.7");
  auto c3_8 = C3(network,
                 weightMap,
                 *conv7->getOutput(0),
                 get_width(768, gw),
                 get_width(768, gw),
                 get_depth(3, gd),
                 true,
                 1,
                 0.5,
                 "model.8");
  auto conv9 = convBlock(network, weightMap, *c3_8->getOutput(0), get_width(1024, gw), 3, 2, 1, "model.9");
  auto spp10 =
      SPP(network, weightMap, *conv9->getOutput(0), get_width(1024, gw), get_width(1024, gw), 3, 5, 7, "model.10");
  auto c3_11 = C3(network,
                  weightMap,
                  *spp10->getOutput(0),
                  get_width(1024, gw),
                  get_width(1024, gw),
                  get_depth(3, gd),
                  false,
                  1,
                  0.5,
                  "model.11");

  /* ------ yolov5 head ------ */
  auto conv12 = convBlock(network, weightMap, *c3_11->getOutput(0), get_width(768, gw), 1, 1, 1, "model.12");
  auto upsample13 = network->addResize(*conv12->getOutput(0));
  assert(upsample13);
  upsample13->setResizeMode(ResizeMode::kNEAREST);
  upsample13->setOutputDimensions(c3_8->getOutput(0)->getDimensions());
  ITensor *inputTensors14[] = {upsample13->getOutput(0), c3_8->getOutput(0)};
  auto cat14 = network->addConcatenation(inputTensors14, 2);
  auto c3_15 = C3(network,
                  weightMap,
                  *cat14->getOutput(0),
                  get_width(1536, gw),
                  get_width(768, gw),
                  get_depth(3, gd),
                  false,
                  1,
                  0.5,
                  "model.15");

  auto conv16 = convBlock(network, weightMap, *c3_15->getOutput(0), get_width(512, gw), 1, 1, 1, "model.16");
  auto upsample17 = network->addResize(*conv16->getOutput(0));
  assert(upsample17);
  upsample17->setResizeMode(ResizeMode::kNEAREST);
  upsample17->setOutputDimensions(c3_6->getOutput(0)->getDimensions());
  ITensor *inputTensors18[] = {upsample17->getOutput(0), c3_6->getOutput(0)};
  auto cat18 = network->addConcatenation(inputTensors18, 2);
  auto c3_19 = C3(network,
                  weightMap,
                  *cat18->getOutput(0),
                  get_width(1024, gw),
                  get_width(512, gw),
                  get_depth(3, gd),
                  false,
                  1,
                  0.5,
                  "model.19");

  auto conv20 = convBlock(network, weightMap, *c3_19->getOutput(0), get_width(256, gw), 1, 1, 1, "model.20");
  auto upsample21 = network->addResize(*conv20->getOutput(0));
  assert(upsample21);
  upsample21->setResizeMode(ResizeMode::kNEAREST);
  upsample21->setOutputDimensions(c3_4->getOutput(0)->getDimensions());
  ITensor *inputTensors21[] = {upsample21->getOutput(0), c3_4->getOutput(0)};
  auto cat22 = network->addConcatenation(inputTensors21, 2);
  auto c3_23 = C3(network,
                  weightMap,
                  *cat22->getOutput(0),
                  get_width(512, gw),
                  get_width(256, gw),
                  get_depth(3, gd),
                  false,
                  1,
                  0.5,
                  "model.23");

  auto conv24 = convBlock(network, weightMap, *c3_23->getOutput(0), get_width(256, gw), 3, 2, 1, "model.24");
  ITensor *inputTensors25[] = {conv24->getOutput(0), conv20->getOutput(0)};
  auto cat25 = network->addConcatenation(inputTensors25, 2);
  auto c3_26 = C3(network,
                  weightMap,
                  *cat25->getOutput(0),
                  get_width(1024, gw),
                  get_width(512, gw),
                  get_depth(3, gd),
                  false,
                  1,
                  0.5,
                  "model.26");

  auto conv27 = convBlock(network, weightMap, *c3_26->getOutput(0), get_width(512, gw), 3, 2, 1, "model.27");
  ITensor *inputTensors28[] = {conv27->getOutput(0), conv16->getOutput(0)};
  auto cat28 = network->addConcatenation(inputTensors28, 2);
  auto c3_29 = C3(network,
                  weightMap,
                  *cat28->getOutput(0),
                  get_width(1536, gw),
                  get_width(768, gw),
                  get_depth(3, gd),
                  false,
                  1,
                  0.5,
                  "model.29");

  auto conv30 = convBlock(network, weightMap, *c3_29->getOutput(0), get_width(768, gw), 3, 2, 1, "model.30");
  ITensor *inputTensors31[] = {conv30->getOutput(0), conv12->getOutput(0)};
  auto cat31 = network->addConcatenation(inputTensors31, 2);
  auto c3_32 = C3(network,
                  weightMap,
                  *cat31->getOutput(0),
                  get_width(2048, gw),
                  get_width(1024, gw),
                  get_depth(3, gd),
                  false,
                  1,
                  0.5,
                  "model.32");

  /* ------ detect ------ */
  IConvolutionLayer *det0 = network->addConvolutionNd(*c3_23->getOutput(0),
                                                      3 * (Yolo::CLASS_NUM + 5),
                                                      DimsHW{1, 1},
                                                      weightMap["model.33.m.0.weight"],
                                                      weightMap["model.33.m.0.bias"]);
  IConvolutionLayer *det1 = network->addConvolutionNd(*c3_26->getOutput(0),
                                                      3 * (Yolo::CLASS_NUM + 5),
                                                      DimsHW{1, 1},
                                                      weightMap["model.33.m.1.weight"],
                                                      weightMap["model.33.m.1.bias"]);
  IConvolutionLayer *det2 = network->addConvolutionNd(*c3_29->getOutput(0),
                                                      3 * (Yolo::CLASS_NUM + 5),
                                                      DimsHW{1, 1},
                                                      weightMap["model.33.m.2.weight"],
                                                      weightMap["model.33.m.2.bias"]);
  IConvolutionLayer *det3 = network->addConvolutionNd(*c3_32->getOutput(0),
                                                      3 * (Yolo::CLASS_NUM + 5),
                                                      DimsHW{1, 1},
                                                      weightMap["model.33.m.3.weight"],
                                                      weightMap["model.33.m.3.bias"]);

  auto yolo = addYoLoLayer(network, weightMap, "model.33", std::vector<IConvolutionLayer *>{det0, det1, det2, det3});
  yolo->getOutput(0)->setName(OUTPUT_BLOB_NAME);
  network->markOutput(*yolo->getOutput(0));

  // Build engine
  builder->setMaxBatchSize(maxBatchSize);
  config->setMaxWorkspaceSize(16 * (1 << 20));  // 16MB
#if defined(USE_FP16)
  config->setFlag(BuilderFlag::kFP16);
#elif defined(USE_INT8)
  std::cout << "Your platform support int8: " << (builder->platformHasFastInt8() ? "true" : "false") << std::endl;
  assert(builder->platformHasFastInt8());
  config->setFlag(BuilderFlag::kINT8);
  Int8EntropyCalibrator2 *calibrator =
      new Int8EntropyCalibrator2(1, INPUT_W, INPUT_H, "./coco_calib/", "int8calib.table", INPUT_BLOB_NAME);
  config->setInt8Calibrator(calibrator);
#endif

  std::cout << "Building engine, please wait for a while..." << std::endl;
  ICudaEngine *engine = builder->buildEngineWithConfig(*network, *config);
  std::cout << "Build engine successfully!" << std::endl;

  // Don't need the network any more
  network->destroy();

  // Release host memory
  for (auto &mem: weightMap) {
    free((void *) (mem.second.values));
  }

  return engine;
}

void YoloV5::APIToModel(unsigned int maxBatchSize,
                        IHostMemory **modelStream,
                        bool &is_p6,
                        float &gd,
                        float &gw,
                        std::string &wts_name) {
  // Create builder
  IBuilder *builder = createInferBuilder(g_logger_);
  IBuilderConfig *config = builder->createBuilderConfig();

  // Create model to populate the network, then set the outputs and create an engine
  ICudaEngine *engine = nullptr;
  if (is_p6) {
    engine = build_engine_p6(maxBatchSize, builder, config, DataType::kFLOAT, gd, gw, wts_name);
  } else {
    engine = build_engine(maxBatchSize, builder, config, DataType::kFLOAT, gd, gw, wts_name);
  }
  assert(engine != nullptr);

  // Serialize the engine
  (*modelStream) = engine->serialize();

  // Close everything down
  engine->destroy();
  builder->destroy();
  config->destroy();
}

cv::Mat &YoloV5::BGR2RGB(float *data, cv::Mat &img) {
  cv::Mat pr_img = preprocess_img(img, INPUT_W, INPUT_H); // letterbox BGR to RGB
  int i = 0;
  for (int row = 0; row < INPUT_H; ++row) {
    uchar *uc_pixel = pr_img.data + row * pr_img.step;
    for (int col = 0; col < INPUT_W; ++col) {
      data[i] = static_cast<float>(uc_pixel[2]) / 255.0;
      data[i + INPUT_H * INPUT_W] = static_cast<float>(uc_pixel[1]) / 255.0;
      data[i + 2 * INPUT_H * INPUT_W] = static_cast<float>(uc_pixel[0]) / 255.0;
      uc_pixel += 3;
      ++i;
    }
  }
  return img;
}

void YoloV5::doInference(IExecutionContext &context,
                         cudaStream_t &stream,
                         void **buffers,
                         float *input,
                         float *output,
                         int batchSize) {
  // DMA input batch data to device, infer on the batch asynchronously, and DMA output back to host
  CUDA_CHECK(cudaMemcpyAsync(buffers[0],
                             input,
                             batchSize * 3 * INPUT_H * INPUT_W * sizeof(float),
                             cudaMemcpyHostToDevice,
                             stream));
  context.enqueue(batchSize, buffers, stream, nullptr);
  CUDA_CHECK(cudaMemcpyAsync(output,
                             buffers[1],
                             batchSize * OUTPUT_SIZE * sizeof(float),
                             cudaMemcpyDeviceToHost,
                             stream));
  cudaStreamSynchronize(stream);
}

void YoloV5::Inference(const std::vector<cv::Scalar> &colors_list,
                       const std::vector<std::string> &id_name,
                       float *data,
                       float *prob,
                       IExecutionContext *context,
                       void **buffers,
                       cudaStream_t &stream,
                       cv::Mat &img) {
  doInference(*context, stream, buffers, data, prob, BATCH_SIZE);
  std::vector<std::vector<Yolo::Detection>> batch_res(1);
  {
    auto &res = batch_res[0];
    nms(res, &prob[0], CONF_THRESH, NMS_THRESH);
  }
  auto &res = batch_res[0];
//  std::vector<Box::BoxRect> boxes;
  for (auto &re: res) {
    cv::Rect r = get_rect(img, re.bbox);
    cv::rectangle(img, r, colors_list[(int) re.class_id], 2);
    cv::putText(img,
        //id_name[std::to_string(static_cast<int>( res[j].class_id))].asString() + " "
        //+ std::to_string(static_cast<int>(res[j].conf * 100)) + "%",
                id_name[static_cast<int>( re.class_id)] + " "
                    + std::to_string(static_cast<int>(re.conf * 100)) + "%",
                cv::Point(r.x, r.y - 1),
                cv::FONT_HERSHEY_PLAIN,
                1.2,
                colors_list[(int) re.class_id],
                2);

//    cv::circle(img, Box::BoxRect(r).getMPoint(), 4, colors_list[static_cast<int>(re.class_id)], -1);
//    boxes.emplace_back(r);
  }
//  return boxes;
}

void YoloV5::Start(int argc, char **argv) {
  if (!ParseArgs(argc, argv)) {
    std::cerr << "arguments not right!" << std::endl;
    std::cerr << "./yolov5 -s [.wts] [.engine] [s/m/l/x/s6/m6/l6/x6 or c/c6 gd gw]  // serialize model to plan file"
              << std::endl;
    std::cerr << "./yolov5 -d [.engine]  // deserialize plan file and run inference" << std::endl;
    exit(-1);
  }

  // create a model using the API directly and serialize it to a stream
  if (!wts_name_.empty()) {
    IHostMemory *modelStream{nullptr};

    APIToModel(BATCH_SIZE, &modelStream, is_p6_, gd_, gw_, wts_name_);
    assert(modelStream != nullptr);

    std::ofstream p(engine_name_, std::ios::binary);
    if (!p) {
      std::cerr << "could not open plan output file" << std::endl;
      exit(-1);
    }
    p.write(reinterpret_cast<const char *>(modelStream->data()), static_cast<long>(modelStream->size()));

    modelStream->destroy();
    exit(0);
  }

  // deserialize the .engine and run inference
  std::ifstream file(engine_name_, std::ios::binary);
  if (!file.good()) {
    std::cerr << "read " << engine_name_ << " error!" << std::endl;
    exit(-1);
  }

  // 移动文件指针，获取文件大小
  file.seekg(0, std::ifstream::end);
  model_size_ = file.tellg();

  // 将文件指针移回头部
  file.seekg(0, std::ifstream::beg);

  // 写入模型数据
  trtModelStream_ = new char[model_size_];
  assert(trtModelStream_);
  file.read(trtModelStream_, model_size_);

  file.close();
}

void YoloV5::Prepare() {
// prepare input data ---------------------------
  data_ = new float[BATCH_SIZE * 3 * INPUT_H * INPUT_W];
  prob_ = new float[BATCH_SIZE * OUTPUT_SIZE];

  runtime_ = createInferRuntime(g_logger_);
  assert(runtime_ != nullptr);

  engine_ = runtime_->deserializeCudaEngine(trtModelStream_, model_size_);
  assert(engine_ != nullptr);

  context_ = engine_->createExecutionContext();
  assert(context_ != nullptr);

  delete[] trtModelStream_;
  trtModelStream_ = nullptr;

  assert(engine_->getNbBindings() == 2);

  // In order to bind the buffers, we need to know the names of the input and output tensors.
  // Note that indices are guaranteed to be less than IEngine::getNbBindings()
  input_index_ = engine_->getBindingIndex(INPUT_BLOB_NAME);
  output_index_ = engine_->getBindingIndex(OUTPUT_BLOB_NAME);
  assert(input_index_ == 0);
  assert(output_index_ == 1);

  // Create GPU buffers on device
  CUDA_CHECK(cudaMalloc(&gpu_buffers_[input_index_], BATCH_SIZE * 3 * INPUT_H * INPUT_W * sizeof(float)));
  CUDA_CHECK(cudaMalloc(&gpu_buffers_[output_index_], BATCH_SIZE * OUTPUT_SIZE * sizeof(float)));

  CUDA_CHECK(cudaStreamCreate(&stream_));
}
