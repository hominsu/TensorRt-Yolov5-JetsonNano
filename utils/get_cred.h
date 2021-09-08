//
// Created by Homin Su on 2021/8/16.
//

#ifndef YOLOV5_UTILS_GET_CRED_H_
#define YOLOV5_UTILS_GET_CRED_H_

#include <grpc++/grpc++.h>

class GetCred {
 public:
  static std::string GetFileContents(const std::string &_path);
  static std::shared_ptr<grpc::ServerCredentials> GetServerCred();
  static std::shared_ptr<grpc::ChannelCredentials> GetClientCred();
};

#endif //YOLOV5_UTILS_GET_CRED_H_
