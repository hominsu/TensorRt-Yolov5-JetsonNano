//
// Created by Homin Su on 2021/9/3.
//

#include <thread>
#include <mutex>
#include <csignal>
#include <sys/signalfd.h>

#include <opencv4/opencv2/opencv.hpp>

#include "model/yolo_v_5.h"
#include "model/gen_color.h"
#include "detect_service/detect_service_server.h"
#include "config/config.h"

// cv::Mat 2 std::string，也可以通过 stringstream 将 cv::Mat 流转换为字符流
// 但是以下方法操作，直接操作内存，快
// TODO: 有没有更优雅的方法？
inline std::string Mat2String(const cv::Mat &img) {
  std::string imag_str;
  std::vector<unsigned char> buff;
  cv::imencode(".jpg", img, buff);
  imag_str.resize(buff.size());
  memcpy(&imag_str[0], buff.data(), buff.size());
  return imag_str;
}

int main(int argc, char **argv) {
  sigset_t mask;  // 掩码
  sigemptyset(&mask); // 清空掩码

  // 添加信号量到掩码
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGQUIT);
  sigaddset(&mask, SIGTERM);

  // 设置该进程为对应的信号集的内容（当前已经的信号集合做并集、交集、覆盖）
  sigprocmask(SIG_BLOCK, &mask, nullptr);

  // 创建 signalfd 句柄，绑定信号
  auto signal_fd = signalfd(-1, &mask, 0);

  // 处理 Parse、Gen engine、Load engine 的 Exception
  try {
    YoloV5::Get()->ParseArgs(argc, argv); // 检查输入参数是否合格
    YoloV5::Get()->GenEngine(); // 生成引擎
    YoloV5::Get()->LoadEngine();  // 加载引擎
  } catch (YoloQuit &_d) {
    std::cout << _d.what();
    return EXIT_SUCCESS;
  } catch (YoloException &_e) {
    std::cerr << _e.what();
    return EXIT_FAILURE;
  }

  // 准备神经网络和 gpu buffer
  YoloV5::Get()->PrepareInput();

  // open the camera
  cv::VideoCapture capture(0);
  if (!capture.isOpened()) {
    std::cout << "Failed to open camera." << std::endl;
    return EXIT_FAILURE;
  }
  capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
  capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

  // create a window to show the detected frame
  cv::namedWindow("dst", cv::WINDOW_AUTOSIZE);

  // 框颜色
  std::vector<cv::Scalar> colors_list = GetColors(YoloV5::CLASS_NUM);
  std::vector<std::string> id_name = {
      "ball"
  };

  // Prepare buffer
  float data[BATCH_SIZE * 3 * YoloV5::INPUT_H * YoloV5::INPUT_W];
  float prob[BATCH_SIZE * YoloV5::OUTPUT_SIZE];

  // 暂存: 检测状态、检测信息、图片的对象
  Info info;

  // 监听地址
  auto local_address =
      Config::Get()->BasicSetting()->local_rpc_address() + ":" + Config::Get()->BasicSetting()->local_rpc_port();

  // rpc 服务对象
  DetectServiceServer server(local_address, info);

  server.Start(); // 启动 rpc 服务

  while (true) {
    cv::Mat img;
    capture >> img;
    if (img.empty()) {
      continue;
    }

    // 转换
    img = YoloV5::BGR2RGB(data, img);

    // 推导（向神经网络传播数据）
    auto boxes = YoloV5::Get()->Inference(colors_list, id_name, data, prob, img);

    // 处理结果
    if (!boxes->empty()) {
      // 格式化成 json 字符串，输出
      for (const auto box: *boxes) {
        std::cout << box.ToJson().dump(4) << std::endl;
      }
      std::cout << std::endl;

      info.set_image(Mat2String(img));
      info.set_rects(boxes);

      // status 的设置一定在 image 和 rect 之前，否则 rpc 会读到无用数据
      info.set_status(true);
    }
    info.set_status(false);

    // 显示图片，检测退出
    cv::imshow("dst", img);
    if (cv::waitKey(1) == 'q') {
      cv::destroyAllWindows();
      break;
    }

    // 读取 signalfd 数据（数据代表信号），检测到对应信号就退出循环
    struct signalfd_siginfo sig_info{};
    auto size = read(signal_fd, &sig_info, sizeof(signalfd_siginfo));
    if (size > 0) {
      if (SIGINT == sig_info.ssi_signo || SIGQUIT == sig_info.ssi_signo || SIGTERM == sig_info.ssi_signo) {
        break;
      } else {
        std::cout << "Read unexpected signal" << std::endl;
        exit(EXIT_FAILURE);
      }
    }

  }

  server.Stop();  // 停止 rpc 服务
  capture.release();  // 释放摄像头
  return EXIT_SUCCESS;
}

