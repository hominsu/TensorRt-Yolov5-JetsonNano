//
// Created by Homin Su on 2021/9/4.
//

#ifndef YOLOV5_DEFS_BOX_H_
#define YOLOV5_DEFS_BOX_H_

#include <nlohmann/json.hpp>
#include <opencv4/opencv2/opencv.hpp>

namespace Box {
struct Rect {
 private:
  int x_;       // x 坐标
  int y_;       // y 坐标
  int width_;   // 宽度
  int height_;  // 高度
  int area_;    // 面积
  int class_id_;   // 检测种类

 public:
  explicit Rect(const nlohmann::json &_j)
      : x_(_j.at("x")),
        y_(_j.at("y")),
        width_(_j.at("width")),
        height_(_j.at("height")),
        area_(_j.at("area")),
        class_id_(_j.at("class_id")) {}

  explicit Rect(const cv::Rect &_rect, int _class_id)
      : x_(_rect.x),
        y_(_rect.y),
        width_(_rect.width),
        height_(_rect.height),
        area_(_rect.area()),
        class_id_(_class_id) {}

  [[nodiscard]] inline nlohmann::json ToJson() const {
    return nlohmann::json{{"x", x_}, {"y", y_}, {"width", width_}, {"height", height_},
                          {"area", area_}, {"class_id", class_id_}};
  }

  [[nodiscard]] inline cv::Point getMPoint() const {
    return {x_ + width_ / 2, y_ + height_ / 2};
  }

  [[nodiscard]] int x() const {
    return x_;
  }
  [[nodiscard]] int y() const {
    return y_;
  }
  [[nodiscard]] int width() const {
    return width_;
  }
  [[nodiscard]] int height() const {
    return height_;
  }
  [[nodiscard]] int area() const {
    return area_;
  }
  [[nodiscard]] int class_id() const {
    return class_id_;
  }
};
}

#endif //YOLOV5_DEFS_BOX_H_
