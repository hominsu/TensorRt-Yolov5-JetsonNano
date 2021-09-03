//
// Created by Homin Su on 2021/9/3.
//

#ifndef YOLOV5_MODEL_YOLO_V_5_H_
#define YOLOV5_MODEL_YOLO_V_5_H_

#include <string>

#include <opencv4/opencv2/opencv.hpp>

#include "../logging.h"
#include "../yololayer.h"

#define USE_FP16  // set USE_INT8 or USE_FP16 or USE_FP32
#define DEVICE 0  // GPU id
#define NMS_THRESH 0.4
#define CONF_THRESH 0.5
#define BATCH_SIZE 1

class YoloV5 {
 public:
  static const int INPUT_H = Yolo::INPUT_H;
  static const int INPUT_W = Yolo::INPUT_W;
  static const int OUTPUT_SIZE = Yolo::MAX_OUTPUT_BBOX_COUNT * sizeof(Yolo::Detection) / sizeof(float)
      + 1;  // we assume the yololayer outputs no more than MAX_OUTPUT_BBOX_COUNT boxes that conf >= 0.1
  const char *INPUT_BLOB_NAME = "data";
  const char *OUTPUT_BLOB_NAME = "prob";

 private:
  std::string wts_name_;    // 权重文件名称
  std::string engine_name_; // 引擎名称
  bool is_p6_ = false;
  float gd_ = 0.0f;
  float gw_ = 0.0f;

  long model_size_{};
  char *trtModelStream_{};

  float *data_{};
  float *prob_{};
  nvinfer1::IRuntime *runtime_{};
  nvinfer1::ICudaEngine *engine_{};
  nvinfer1::IExecutionContext *context_{};
  int input_index_{};
  int output_index_{};
  void *gpu_buffers_[2]{};
  cudaStream_t stream_{};

  Logger g_logger_;

 public:
  ~YoloV5();
  static YoloV5 *Get() {
    static YoloV5 v;
    return &v;
  }
 private:
  YoloV5();

 public:
  void Start(int argc, char **argv);
  void Prepare();
  inline void doInference(nvinfer1::IExecutionContext &context,
                          cudaStream_t &stream,
                          void **buffers,
                          float *input,
                          float *output,
                          int batchSize);
  void Inference(const std::vector<cv::Scalar> &colors_list,
                 const std::vector<std::string> &id_name,
                 float *data,
                 float *prob,
                 nvinfer1::IExecutionContext *context,
                 void **buffers,
                 cudaStream_t &stream,
                 cv::Mat &img);

 private:
  bool ParseArgs(int argc, char **argv);
  static inline int get_width(int x, float gw, int divisor = 8);
  static inline int get_depth(int x, float gd);
  static inline cv::Mat &BGR2RGB(float *data, cv::Mat &img);

  nvinfer1::ICudaEngine *build_engine(unsigned int maxBatchSize,
                                      nvinfer1::IBuilder *builder,
                                      nvinfer1::IBuilderConfig *config,
                                      nvinfer1::DataType dt,
                                      float &gd,
                                      float &gw,
                                      std::string &wts_name);
  nvinfer1::ICudaEngine *build_engine_p6(unsigned int maxBatchSize,
                                         nvinfer1::IBuilder *builder,
                                         nvinfer1::IBuilderConfig *config,
                                         nvinfer1::DataType dt,
                                         float &gd,
                                         float &gw,
                                         std::string &wts_name);
  void APIToModel(unsigned int maxBatchSize,
                  nvinfer1::IHostMemory **modelStream,
                  bool &is_p6,
                  float &gd,
                  float &gw,
                  std::string &wts_name);
};

#endif //YOLOV5_MODEL_YOLO_V_5_H_
