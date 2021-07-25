//
// Created by HominSu on 2021/7/22.
//

#ifndef SYSTEM_HTTP_SERVER_H_
#define SYSTEM_HTTP_SERVER_H_

#include <event2/event.h>
#include <event2/buffer.h>

#include <string>

#include "../x_thread.h"
#include "../defs/send_struct.h"

struct ServerCtx {
  event_base *base;
  std::string str;
  void *this_ptr;
  void *controller_ptr;
};

struct ClientCtx {
  event_base *base;
  std::string str;
};

class Server : public XThread {
 private:
  std::string uuid_{};

 public:
  ~Server();
  /**
   * @brief 单件模式，只初始化一次
   * @return Server *
   */
  static Server *Get() {
    static Server s;
    return &s;
  }
 private:
  Server();

 public:
  void Main() override;
  std::string Send(const Api::Body &_body);

 private:
  static void http_server_call_back(struct evhttp_request *request, void *_arg);
  static std::string getEvBufferString(evbuffer *in_buffer);
  void DetectImageGetRequest(evhttp_request *_request, const ServerCtx *_arg);
  static void SetUUIDPostRequest(evhttp_request *_request, const ServerCtx *_arg);

  static void http_client_call_back(struct evhttp_request *_req, void *_ctx);
};

#endif //SYSTEM_HTTP_SERVER_H_
