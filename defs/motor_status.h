//
// Created by HominSu on 2021/7/23.
//

#ifndef SYSTEM_DEFS_MOTOR_STATUS_H_
#define SYSTEM_DEFS_MOTOR_STATUS_H_

namespace MotorInfo {
struct Sta {
  bool fault;
  bool enabling;
  bool running;
  bool instruction_completion;
  bool path_completion;
  bool zero_completion;
};

struct Status {
  Sta axis_1;
  Sta axis_2;
  Sta axis_3;
  Sta axis_4;
  Sta axis_5;
  Sta axis_6;
};

struct Command {
  int axis_1;
  int axis_2;
  int axis_3;
  int axis_4;
  int axis_5;
  int axis_6;
};

struct Real {
  int axis_1;
  int axis_2;
  int axis_3;
  int axis_4;
  int axis_5;
  int axis_6;
};

struct Info {
  Status motor_status;
  Command command_pos;
  Real current_pos;
};

void to_json(nlohmann::json &j, const Sta &_sta) {
  j = nlohmann::json{{"fault", _sta.fault}, {"enabling", _sta.enabling}, {"running", _sta.running},
                     {"instruction_completion", _sta.instruction_completion},
                     {"path_completion", _sta.path_completion}, {"zero_completion", _sta.zero_completion}};
}

void from_json(const nlohmann::json &j, Sta &_sta) {
  j.at("fault").get_to(_sta.fault);
  j.at("enabling").get_to(_sta.enabling);
  j.at("running").get_to(_sta.running);
  j.at("instruction_completion").get_to(_sta.instruction_completion);
  j.at("path_completion").get_to(_sta.path_completion);
  j.at("zero_completion").get_to(_sta.zero_completion);
}

void to_json(nlohmann::json &j, const Status &_status) {
  j = nlohmann::json{{"axis_1", _status.axis_1}, {"axis_2", _status.axis_2}, {"axis_3", _status.axis_3},
                     {"axis_4", _status.axis_4}, {"axis_5", _status.axis_5}, {"axis_6", _status.axis_6}};
}

void from_json(const nlohmann::json &j, Status &_status) {
  j.at("axis_1").get_to(_status.axis_1);
  j.at("axis_2").get_to(_status.axis_2);
  j.at("axis_3").get_to(_status.axis_3);
  j.at("axis_4").get_to(_status.axis_4);
  j.at("axis_5").get_to(_status.axis_5);
  j.at("axis_6").get_to(_status.axis_6);
}

void to_json(nlohmann::json &j, const Command &_command) {
  j = nlohmann::json{{"axis_1", _command.axis_1}, {"axis_2", _command.axis_2}, {"axis_3", _command.axis_3},
                     {"axis_4", _command.axis_4}, {"axis_5", _command.axis_5}, {"axis_6", _command.axis_6}};
}

void from_json(const nlohmann::json &j, Command &_command) {
  j.at("axis_1").get_to(_command.axis_1);
  j.at("axis_2").get_to(_command.axis_2);
  j.at("axis_3").get_to(_command.axis_3);
  j.at("axis_4").get_to(_command.axis_4);
  j.at("axis_5").get_to(_command.axis_5);
  j.at("axis_6").get_to(_command.axis_6);
}

void to_json(nlohmann::json &j, const Real &_real) {
  j = nlohmann::json{{"axis_1", _real.axis_1}, {"axis_2", _real.axis_2}, {"axis_3", _real.axis_3},
                     {"axis_4", _real.axis_4}, {"axis_5", _real.axis_5}, {"axis_6", _real.axis_6}};
}

void from_json(const nlohmann::json &j, Real &_real) {
  j.at("axis_1").get_to(_real.axis_1);
  j.at("axis_2").get_to(_real.axis_2);
  j.at("axis_3").get_to(_real.axis_3);
  j.at("axis_4").get_to(_real.axis_4);
  j.at("axis_5").get_to(_real.axis_5);
  j.at("axis_6").get_to(_real.axis_6);
}

void to_json(nlohmann::json &j, const Info &_info) {
  j = nlohmann::json{{"motor_status", _info.motor_status}, {"command_pos", _info.command_pos},
                     {"current_pos", _info.current_pos}};
}

void from_json(const nlohmann::json &j, Info &_info) {
  j.at("motor_status").get_to(_info.motor_status);
  j.at("command_pos").get_to(_info.command_pos);
  j.at("current_pos").get_to(_info.current_pos);
}

}

#endif //SYSTEM_DEFS_MOTOR_STATUS_H_
