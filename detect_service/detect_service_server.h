//
// Created by Homin Su on 2021/9/6.
//

#ifndef YOLOV5_DETECT_SERVICE_DETECT_SERVICE_SERVER_H_
#define YOLOV5_DETECT_SERVICE_DETECT_SERVICE_SERVER_H_

#include <grpc++/grpc++.h>
#include "../util/x_thread.h"
#include "../defs/info.h"

class DetectServiceServer : public XThread {
 private:
  std::string local_address_{};  ///< rpc 服务地址：ip+端口
  std::unique_ptr<grpc::Server> server_{};  ///< rpc 服务句柄，用智能指针管理
  Info &info_;

 public:
  explicit DetectServiceServer(std::string _local_address, Info &info)
      : local_address_(std::move(_local_address)), info_(info) {};

 public:
  void Main() override;
  void Stop() override;
};

#endif //YOLOV5_DETECT_SERVICE_DETECT_SERVICE_SERVER_H_
