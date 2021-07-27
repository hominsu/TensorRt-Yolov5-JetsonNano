//
// Created by hominsu on 2021/7/25.
//

#ifndef YOLOLAYER_H_DEFS_RESPONSE_STRUCT_H_
#define YOLOLAYER_H_DEFS_RESPONSE_STRUCT_H_

#include <nlohmann/json.hpp>

struct Response {
  std::string cv_mat;
  std::string boxes;

  Response(const std::string &cv_mat, const std::string &boxes) : cv_mat(cv_mat), boxes(boxes) {}

  void to_json(nlohmann::json &j, const Response &_resp) {
    j = nlohmann::json{{"cv_mat", _resp.cv_mat}, {"boxes", _resp.boxes}};
  }

  void from_json(const nlohmann::json &j, Response &_resp) {
    j.at("cv_mat").get_to(_resp.cv_mat);
    j.at("boxes").get_to(_resp.boxes);
  }

};

#endif //YOLOLAYER_H_DEFS_RESPONSE_STRUCT_H_
