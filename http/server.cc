//
// Created by HominSu on 2021/7/22.
//

#include <event2/http.h>
#include <event2/bufferevent.h>
#include <event2/keyvalq_struct.h>

#include <iostream>
#include <csignal>
#include <cstring>
#include <sstream>

#include "server.h"

Server::Server() = default;

Server::~Server() = default;

std::string Server::getEvBufferString(evbuffer *in_buffer) {
  char buffer[1024]{0};
  std::stringstream ss;
  while (evbuffer_get_length(in_buffer)) {
    int len = evbuffer_remove(in_buffer, buffer, sizeof(buffer) - 1);
    if (len > 0) {
      buffer[len] = 0;
      ss << buffer;
    }
  }
  std::string str = ss.str();
  return str;
}

void Server::DetectImageGetRequest(evhttp_request *_request, const ServerCtx *_arg) {
  auto t_ptr = static_cast<Server *>(_arg->this_ptr);
  if (t_ptr->uuid_.empty()) {
    evhttp_send_reply(_request, HTTP_BADREQUEST, "Not Register UUID", nullptr);
    return;
  }

  t_ptr->SetImageCanRead(false);
  t_ptr->SetDetect(true);

  while (!t_ptr->IsImageCanRead()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  Response resp{t_ptr->GetCvMatStr(), t_ptr->GetBoxesStr()};
  nlohmann::json js_obj;
  resp.to_json(js_obj, resp);
  auto data = js_obj.dump();

  auto data_buf = evbuffer_new();
  evbuffer_add(data_buf, data.c_str(), data.size());

  evhttp_send_reply(_request, HTTP_OK, "Command Received Success", data_buf);
  evbuffer_free(data_buf);
}

void Server::SetUUIDPostRequest(evhttp_request *_request, const ServerCtx *_arg) {
  auto in_buffer = evhttp_request_get_input_buffer(_request);
  auto str = getEvBufferString(in_buffer);

  if (!str.empty()) {
    auto t_ptr = static_cast<Server *>(_arg->this_ptr);
    t_ptr->uuid_ = str;
    evhttp_send_reply(_request, HTTP_OK, "Set UUID Success", nullptr);
  } else {
    evhttp_send_reply(_request, HTTP_BADREQUEST, "", nullptr);
  }
}

void Server::http_server_call_back(struct evhttp_request *request, void *_arg) {
#if DEBUG
  std::cout << std::endl << "===============" << std::endl << "http_cal_back" << std::endl;
#endif

  auto arg = static_cast<ServerCtx *>(_arg);

  // 获取浏览器的请求信息
  // url 请求的资源地址
  const char *uri = evhttp_request_get_uri(request);
#if DEBUG
  std::cout << "url: " << url << std::endl;
#endif

  // 请求类型 GET POST
  auto request_command_type = evhttp_request_get_command(request);

#if DEBUG
  // 获取消息报头
  auto headers = evhttp_request_get_input_headers(request);
  std::cout << "headers: " << std::endl;
  for (evkeyval *p_evkeyval = headers->tqh_first; p_evkeyval != nullptr; p_evkeyval = p_evkeyval->next.tqe_next) {
    std::cout << "\t" << p_evkeyval->key << ": " << p_evkeyval->value << std::endl;
  }
  std::cout << std::endl;

  // 获取请求正文 (GET 为空，POST 有表单信息)
  auto in_buffer = evhttp_request_get_input_buffer(request);
  char buffer[1024]{0};
  std::cout << "input data: " << std::endl;
  while (evbuffer_get_length(in_buffer)) {
    int len = evbuffer_remove(in_buffer, buffer, sizeof(buffer) - 1);
    if (len > 0) {
      buffer[len] = '\0';
      std::cout << buffer << std::endl;
    }
  }

  std::cout << "===============" << std::endl;
#endif

  // 回应浏览器
  // 状态行 消息报头 响应正文

  // 分析请求的 url
  if (0 == strcmp(uri, "/detectImage") && EVHTTP_REQ_GET == request_command_type) {
    DetectImageGetRequest(request, arg);
  } else if (0 == strcmp(uri, "/setUUID") && EVHTTP_REQ_POST == request_command_type) {
    SetUUIDPostRequest(request, arg);
  } else {
    evhttp_send_reply(request, HTTP_NOTFOUND, "", nullptr);
  }

  // TODO: 修复这个问题: 当线程关闭时退出 event_base_loop，但是退出的时候不连接 server 触发回调函数就不会停止
  auto this_p = static_cast<Server *>(arg->this_ptr);
  if (!this_p->is_running()) {
    event_base_loopbreak(arg->base);
  }
}

void Server::Main() {
#ifdef _WIN32
  // 初始化 socket
  WSADATA wsa;
  WSAStartup(MAKEWORD(2, 2), &wsa);
#else
  // 忽略管道信号，发送数据给已经关闭的 socket
  if (SIG_ERR == signal(SIGPIPE, SIG_IGN))
    return;
#endif

  ServerCtx ctx{event_base_new(), "", this};

  // http 服务器
  // 创建 evhttp 上下文
  auto ev_http = evhttp_new(ctx.base);

  // 绑定端口和 IP
  if (0 != evhttp_bind_socket(ev_http, "0.0.0.0", 8080)) {
    std::cout << "evhttp_bind_socket failed" << std::endl;
  }

  // 设置回调函数
  evhttp_set_gencb(ev_http, http_server_call_back, &ctx);

  // 进入事件主循环
  if (ctx.base) {
    event_base_dispatch(ctx.base);
  }

  // 释放
  if (ev_http) {
    evhttp_free(ev_http);
  }
  if (ctx.base) {
    event_base_free(ctx.base);
  }
}

// ---------------------------------------- Client ----------------------------------------

void Server::http_client_call_back(struct evhttp_request *_req, void *_ctx) {
#if DEBUG
  std::cout << "http_client_call_back" << std::endl;
#endif
  auto ctx = static_cast<ClientCtx *>(_ctx);

  // 服务端响应错误
  if (nullptr == _req) {
#if DEBUG
    std::cerr << "socket error: " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()) << std::endl;
#endif
    return;
  }

#if DEBUG
  // 获取 path
  auto path = evhttp_request_get_uri(_req);
  std::cout << "request path is " << path << std::endl;

  std::string file_path = "..";
  file_path += path;
  std::cout << "file_path: " << file_path << std::endl;

  // 获取返回的 code 200 404
  std::cout << "Response: " << evhttp_request_get_response_code(_req);
  std::cout << " " << evhttp_request_get_response_code_line(_req) << std::endl;
#endif

  std::stringstream ss;
  char buffer[1024]{0};
  while (true) {
    int len = evbuffer_remove(evhttp_request_get_input_buffer(_req), buffer, sizeof(buffer) - 1);
    if (len <= 0) {
      break;
    }
    buffer[len] = 0;
    ss << buffer;
  }
  ctx->str = ss.str();
  event_base_loopbreak(ctx->base);
}

std::string Server::Send(const Api::Body &_body) {
  ClientCtx ctx{event_base_new(), ""};
#ifdef _WIN32
  // 初始化 socket
  WSADATA wsa;
  WSAStartup(MAKEWORD(2, 2), &wsa);
#else
  // 忽略管道信号，发送数据给已经关闭的 socket
  if (SIG_ERR == signal(SIGPIPE, SIG_IGN))
    return "";
#endif

  // 分析 url 地址
  // url
  auto uri = evhttp_uri_parse(_body.url.c_str());

  // 添加 uuid
  if (!uuid_.empty()) {
    evhttp_uri_set_query(uri, std::string("uuid=" + uuid_).c_str());
  }

  // http https
  auto scheme = evhttp_uri_get_scheme(uri);
  if (nullptr == scheme) {
    std::cerr << "scheme is null" << std::endl;
    return "";
  }
#if DEBUG
  std::cout << "scheme is " << scheme << std::endl;
#endif

  // port
  auto port = evhttp_uri_get_port(uri);
  if (port < 0) {
    if (0 == strcmp(scheme, "http")) {
      port = 80;
    } else if (0 == strcmp(scheme, "https")) {
      port = 443;
    }
  }
#if DEBUG
  std::cout << "port is " << port << std::endl;
#endif

  // host
  auto host = evhttp_uri_get_host(uri);
  if (nullptr == host) {
#if DEBUG
    std::cerr << "host is " << _body.url << std::endl;
#endif
    return "";
  }
#if DEBUG
  std::cout << "host is " << host << std::endl;
#endif

  // path
  auto path = evhttp_uri_get_path(uri);
  if (nullptr == path || 0 == strlen(path)) {
    path = "/";
  }
#if DEBUG
  std::cout << "path is " << path << std::endl;

  // 获得请求信息
  auto query = evhttp_uri_get_query(uri);
  if (nullptr == query) {
    std::cout << "query is nullptr" << std::endl;
  }
#endif

  // bufferevent 连接 http 服务
  auto bev = bufferevent_socket_new(ctx.base, -1, BEV_OPT_CLOSE_ON_FREE);
  auto ev_con = evhttp_connection_base_bufferevent_new(ctx.base, nullptr, bev, host, port);

  // http client 请求
  auto req = evhttp_request_new(http_client_call_back, &ctx);

  // 设置请求的 head 消息报头信息
  auto out_headers = evhttp_request_get_output_headers(req);
  evhttp_add_header(out_headers, "Host", host);

  // 发起请求
  if (_body.method == "GET") {
    evhttp_make_request(ev_con, req, EVHTTP_REQ_GET, path);
  } else if (_body.method == "POST") {
    auto output = evhttp_request_get_output_buffer(req);
    evbuffer_add(output, _body.request_body.c_str(), _body.request_body.size());
    evhttp_make_request(ev_con, req, EVHTTP_REQ_POST, path);
  }

  // 进入事件主循环
  if (ctx.base) {
    event_base_dispatch(ctx.base);
  }

  // 释放
  if (ctx.base) {
    event_base_free(ctx.base);
  }

  return ctx.str;
}

bool Server::IsDetect() const {
  return detect_;
}

void Server::SetDetect(bool detect) {
  detect_ = detect;
}

bool Server::IsImageCanRead() const {
  return image_can_read_;
}

void Server::SetImageCanRead(bool image_can_read) {
  image_can_read_ = image_can_read;
}

const std::string &Server::GetCvMatStr() const {
  return cv_mat_str_;
}

void Server::SetCvMatStr(const std::string &cv_mat_str) {
  cv_mat_str_ = cv_mat_str;
}

const std::string &Server::GetBoxesStr() const {
  return boxes_str_;
}

void Server::SetBoxesStr(const std::string &boxes_str) {
  boxes_str_ = boxes_str;
}
