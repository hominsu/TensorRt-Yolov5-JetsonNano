//
// Created by Homin Su on 2021/9/3.
//

#ifndef YOLOV5_MODEL_YOLO_EXCEPTION_H_
#define YOLOV5_MODEL_YOLO_EXCEPTION_H_

#include <string>
#include <exception>
#include <utility>

class YoloException : public std::exception {
 private:
  std::string message_;

 public:
  YoloException() noexcept: message_("YoloException: Error.") {}

  explicit YoloException(std::string str) noexcept: message_(std::move(str)) {}

  YoloException(const YoloException &e) noexcept {
    message_ = e.message_;
  }

  ~YoloException() noexcept override = default;

  [[nodiscard]] const char *what() const noexcept override {
    return message_.c_str();
  }
};

#endif //YOLOV5_MODEL_YOLO_EXCEPTION_H_
