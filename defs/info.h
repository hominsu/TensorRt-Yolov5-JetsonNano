//
// Created by Homin Su on 2021/9/7.
//

#ifndef YOLOV5_DEFS_INFO_H_
#define YOLOV5_DEFS_INFO_H_

#include <shared_mutex>

#include "box.h"

class Info {
 private:
  bool status_ = false;
  std::shared_ptr<std::vector<Box::Rect>> rects_;
  std::string image_{};
  mutable std::shared_mutex mutex_;

 public:
  Info() = default;
  Info(bool status,
       std::shared_ptr<std::vector<Box::Rect>> rects,
       std::string image) : status_(status), rects_(std::move(rects)), image_(std::move(image)) {}

  bool status() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return status_;
  }
  void set_status(bool status) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    status_ = status;
  }
  const std::shared_ptr<std::vector<Box::Rect>> &rects() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return rects_;
  }
  void set_rects(const std::shared_ptr<std::vector<Box::Rect>> &rects) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    rects_ = rects;
  }
  const std::string &image() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return image_;
  }
  void set_image(const std::string &image) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    image_ = image;
  }
};

#endif //YOLOV5_DEFS_INFO_H_
