//
// Created by Homin Su on 2021/9/6.
//

#include "detect_service_impl.h"

::grpc::Status DetectServiceImpl::DetectedRect(::grpc::ServerContext *_context,
                                               const ::Detect::DetectRequest *_request,
                                               ::Detect::DetectResponse *_response) {
  // 写入检测状态，如果图片中检测到对象即为 true
  _response->set_status(info_.status());

  if (_request->status()) {
    // 如果检测到，继续写入 rect 和 image
    if (info_.status()) {
      // 写入图片
      _response->set_image(info_.image());

      // 写入检测结果
      for (const auto &r: *(info_.rects())) {
        auto rect = _response->add_rect();
        rect->set_x(r.x());
        rect->set_y(r.y());
        rect->set_width(r.width());
        rect->set_height(r.height());
        rect->set_area(r.area());
        rect->set_class_id(r.class_id());
      }
    }
  }
  return ::grpc::Status::OK;
}