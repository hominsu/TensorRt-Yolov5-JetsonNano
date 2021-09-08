//
// Created by Homin Su on 2021/9/6.
//

#ifndef YOLOV5_CONFIG_CONFIG_H_
#define YOLOV5_CONFIG_CONFIG_H_

#include <memory>

#include "basic_setting.h"

class Config {
 private:
  std::unique_ptr<Basic::Setting> basic_setting_;

 public:
  ~Config();

  /**
   * @brief 单件模式
   * @return Config *
   */
  static Config *Get() {
    static Config c;
    return &c;
  }

 private:
  Config();
  void Init();

 public:
  [[nodiscard]] const std::unique_ptr<Basic::Setting> &BasicSetting() const;
};

#endif //YOLOV5_CONFIG_CONFIG_H_
