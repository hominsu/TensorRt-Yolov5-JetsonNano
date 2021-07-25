//
// Created by HominSu on 2021/7/22.
//

#ifndef SYSTEM_DEFS_MOTION_H_
#define SYSTEM_DEFS_MOTION_H_

#include <nlohmann/json.hpp>

namespace Motion {
struct Zero {
  bool status;

  static void to_json(nlohmann::json &j, const Zero &_zero) {
    j = nlohmann::json{{"status", _zero.status}};
  }

  static void from_json(const nlohmann::json &j, Zero &_zero) {
    j.at("status").get_to(_zero.status);
  }
};

struct Pos {
  double x;
  double y;
  double z;
  double rx;
  double ry;

  static void to_json(nlohmann::json &j, const Pos &_coordinate) {
    j = nlohmann::json{{"x", _coordinate.x}, {"y", _coordinate.y}, {"z", _coordinate.y},
                       {"rx", _coordinate.rx}, {"ry", _coordinate.ry}};
  }

  static void from_json(const nlohmann::json &j, Pos &_coordinate) {
    j.at("x").get_to(_coordinate.x);
    j.at("y").get_to(_coordinate.y);
    j.at("z").get_to(_coordinate.z);
    j.at("rx").get_to(_coordinate.rx);
    j.at("ry").get_to(_coordinate.ry);
  }
};

struct Done {
  bool status;

  static void to_json(nlohmann::json &j, const Done &_done) {
    j = nlohmann::json{{"status", _done.status}};
  }

  static void from_json(const nlohmann::json &j, Done &_done) {
    j.at("status").get_to(_done.status);
  }
};

}

#endif //SYSTEM_DEFS_MOTION_H_
