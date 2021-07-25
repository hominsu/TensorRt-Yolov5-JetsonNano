//
// Created by HominSu on 2021/7/22.
//

#ifndef YOLOLAYER_H_DEFS_SEND_STRUCT_H_
#define YOLOLAYER_H_DEFS_SEND_STRUCT_H_

#include <nlohmann/json.hpp>

namespace Api {
struct Body {
  std::string url;
  std::string method;
  std::string request_body;

  static void to_json(nlohmann::json &j, const Body &_body) {
    j = nlohmann::json{{"url", _body.url}, {"method", _body.method}, {"request_body", _body.request_body}};
  }

  static void from_json(const nlohmann::json &j, Body &_body) {
    j.at("url").get_to(_body.url);
    j.at("method").get_to(_body.method);
    j.at("request_body").get_to(_body.request_body);
  }
};
}

#endif //YOLOLAYER_H_DEFS_SEND_STRUCT_H_
