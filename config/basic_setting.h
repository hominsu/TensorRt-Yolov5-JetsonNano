//
// Created by Homin Su on 2021/9/6.
//

#ifndef YOLOV5_CONFIG_BASIC_SETTING_H_
#define YOLOV5_CONFIG_BASIC_SETTING_H_

#include <nlohmann/json.hpp>

namespace Basic {
/**
 * @brief 基础设置
 */
struct Setting {
 private:
  std::string uuid_;                // json: "uuid"
  std::string rtmp_server_address_;  // json: "server_rpc_address"
  std::string rtmp_server_port_;     // json: "server_rpc_port"
  std::string local_rpc_address_;   // json: "local_rpc_address"
  std::string local_rpc_port_;      // json: "local_rpc_port"

 public:
  explicit Setting(const nlohmann::json &j)
      : uuid_(j.at("uuid")),
        server_rpc_address_(j.at("rtmp_server_address")),
        server_rpc_port_(j.at("rtmp_server_port")),
        local_rpc_address_(j.at("local_rpc_address")),
        local_rpc_port_(j.at("local_rpc_port")) {};

  [[nodiscard]] const std::string &uuid() const {
    return uuid_;
  }
  [[nodiscard]] const std::string &rtmp_server_address() const {
    return rtmp_server_address_;
  }
  [[nodiscard]] const std::string &rtmp_server_port() const {
    return rtmp_server_port_;
  }
  [[nodiscard]] const std::string &local_rpc_address() const {
    return local_rpc_address_;
  }
  [[nodiscard]] const std::string &local_rpc_port() const {
    return local_rpc_port_;
  }
};
}

#endif //YOLOV5_CONFIG_BASIC_SETTING_H_
