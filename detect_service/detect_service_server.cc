//
// Created by Homin Su on 2021/9/6.
//

#include "detect_service_server.h"
#include "detect_service_impl.h"
#include "../util/get_cred.h"

void DetectServiceServer::Stop() {
  std::function<void()> func = [this]() {
    this->server_->Shutdown();
  };
  XThread::StopWith(func);
}

void DetectServiceServer::Main() {
  DetectServiceImpl service(info_);
  grpc::ServerBuilder builder;

  builder.AddListeningPort(local_address_, GetCred::GetServerCred());
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  server_ = std::move(server);

#if DEBUG
  std::cout << "Server listening on " << local_address_ << std::endl;
#endif

  server_->Wait();

#if DEBUG
  std::cout << "Rpc Service Shut Down" << std::endl;
#endif
}

