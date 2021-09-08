//
// Created by Homin Su on 2021/9/6.
//

#ifndef YOLOV5_DETECT_SERVICE_DETECT_SERVICE_IMPL_H_
#define YOLOV5_DETECT_SERVICE_DETECT_SERVICE_IMPL_H_

#include <vector>

#include <grpc++/grpc++.h>
#include "detect.grpc.pb.h"
#include "../defs/box.h"
#include "../defs/info.h"

class DetectServiceImpl final : public Detect::DetectResultService::Service {
 private:
  Info &info_;

 public:
  explicit DetectServiceImpl(Info &info) : info_(info) {}

  ::grpc::Status DetectedRect(::grpc::ServerContext *_context,
                              const ::Detect::DetectRequest *_request,
                              ::Detect::DetectResponse *_response) override;
};

#endif //YOLOV5_DETECT_SERVICE_DETECT_SERVICE_IMPL_H_
