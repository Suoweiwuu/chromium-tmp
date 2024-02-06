#ifndef CONTENT_PUBLIC_BROWSER_DONG_SHANG_WEBSOCKET_CLIENT_H_
#define CONTENT_PUBLIC_BROWSER_DONG_SHANG_WEBSOCKET_CLIENT_H_

#if defined(MY_BASE_IMPLEMENTATION)
#define MY_EXPORT __declspec(dllexport)
#else
#define MY_EXPORT __declspec(dllimport)
#endif  // defined(BASE_IMPLEMENTATION)

#include <string>
#include "content/public/browser/dongshang/websocket.h"
#include "content/public/browser/dongshang/websocket_listener.h"


class WebsocketClient {
 public:
  WebsocketClient(std::string url, WebSocketListener* listener);
  ~WebsocketClient();

  WebsocketClient(const WebsocketClient&) = delete;
  WebsocketClient& operator=(const WebsocketClient&) = delete;

  MY_EXPORT bool Connect();
  MY_EXPORT bool Disconnect();
  void Send(std::string message);

 private:
  bool CreateWebSocket();

  std::unique_ptr<WebSocket> socket_;
  std::string url_;
  WebSocketListener* listener_;
};

#endif  // CONTENT_PUBLIC_BROWSER_DONG_SHANG_WEBSOCKET_CLIENT_H_
