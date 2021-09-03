//
// Created by Homin Su on 2021/9/3.
//

#include <thread>

#include <opencv4/opencv2/opencv.hpp>

#include "model/yolo_v_5.h"
#include "gen_color.h"

static const int CLASS_NUM = Yolo::CLASS_NUM;

int main(int argc, char **argv) {
  YoloV5::Get()->Start(argc, argv);

  YoloV5::Get()->Prepare();

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
  std::vector<cv::Scalar> colors_list = GetColors(CLASS_NUM);
  std::vector<std::string> id_name = {
      "ball"
  };
  // ----------------------------------

  while (true) {
    cv::Mat img;
    capture >> img;
    if (img.empty()) {
      continue;
    }

//    img = BGR2RGB(data, img);
//
//    Inference(colors_list, id_name, data, prob, context, buffers, stream, img);
//
//    cv::imshow("dst", img);
//    if (cv::waitKey(1) == 'q') {
//      cv::destroyAllWindows();
//      break;
//    }
  }

  capture.release();
  return 0;
}

