//
// Created by HominSu on 2021/7/22.
//

#ifndef SYSTEM_DEFS_BOX_H_
#define SYSTEM_DEFS_BOX_H_

#include <opencv4/opencv2/opencv.hpp>
#include <nlohmann/json.hpp>

/**
 * 保存 rect，并对其进行序列化与反序列化
 */
namespace Box {
struct BoxRect {
  int x{};
  int y{};
  int width{};
  int height{};
  int area{};

  BoxRect(int x, int y, int width, int height, int area) : x(x), y(y), width(width), height(height), area(area) {}

  explicit BoxRect(const cv::Rect &_rect)
      : x(_rect.x), y(_rect.y), width(_rect.width), height(_rect.height), area(_rect.area()) {}

  [[nodiscard]] inline cv::Point getMPoint() const {
    return {x + width / 2, y + height / 2};
  }

};

void to_json(nlohmann::json &j, const BoxRect &_box_info) {
  j = nlohmann::json{{"x", _box_info.x}, {"y", _box_info.y}, {"width", _box_info.width}, {"height", _box_info.height},
                     {"area", _box_info.area}};
}

void from_json(const nlohmann::json &j, BoxRect &_box_info) {
  j.at("x").get_to(_box_info.x);
  j.at("y").get_to(_box_info.y);
  j.at("width").get_to(_box_info.width);
  j.at("height").get_to(_box_info.height);
  j.at("area").get_to(_box_info.area);
}
}

#endif //SYSTEM_DEFS_BOX_H_
