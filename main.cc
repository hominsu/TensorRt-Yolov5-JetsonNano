//
// Created by Homin Su on 2021/9/3.
//

#include <thread>

#include <opencv4/opencv2/opencv.hpp>

#include "model/yolo_v_5.h"
#include "model/gen_color.h"

int main(int argc, char **argv) {
  try {
    YoloV5::Get()->ParseArgs(argc, argv);
    YoloV5::Get()->GenEngine();
    YoloV5::Get()->LoadEngine();
  } catch (YoloException &_e) {
    std::cerr << _e.what();
    return -1;
  }

  YoloV5::Get()->PrepareInput();

  // open the camera
  cv::VideoCapture capture(0);
  if (!capture.isOpened()) {
    std::cout << "Failed to open camera." << std::endl;
    return -1;
  }
  capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
  capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

  // create a window to show the detected frame
  cv::namedWindow("dst", cv::WINDOW_AUTOSIZE);

  // ----------------------------------
  std::vector<cv::Scalar> colors_list = GetColors(YoloV5::CLASS_NUM);
  std::vector<std::string> id_name = {
      "ball"
  };
  // ----------------------------------

  float data[BATCH_SIZE * 3 * YoloV5::INPUT_H * YoloV5::INPUT_W];
  float prob[BATCH_SIZE * YoloV5::OUTPUT_SIZE];

  while (true) {
    cv::Mat img;
    capture >> img;
    if (img.empty()) {
      continue;
    }

    img = YoloV5::BGR2RGB(data, img);

    YoloV5::Get()->Inference(colors_list, id_name, data, prob, img);

    cv::imshow("dst", img);
    if (cv::waitKey(1) == 'q') {
      cv::destroyAllWindows();
      break;
    }
  }

  capture.release();
  return 0;
}

