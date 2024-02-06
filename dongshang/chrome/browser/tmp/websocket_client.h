#ifndef DONG_SHANG_CHROME_BROWSER_WEBSOCKET_CLIENT_H_
#define DONG_SHANG_CHROME_BROWSER_WEBSOCKET_CLIENT_H_

#include <string>
#include "dongshang/chrome/browser/websocket.h"
#include "dongshang/chrome/browser/websocket_listener.h"


class WebsocketClient {
 public:
  WebsocketClient(std::string url, WebSocketListener* listener);
  ~WebsocketClient();

  WebsocketClient(const WebsocketClient&) = delete;
  WebsocketClient& operator=(const WebsocketClient&) = delete;

  bool Connect();
  bool Disconnect();
  void Send(std::string message);

 private:
  bool CreateWebSocket();

  std::unique_ptr<WebSocket> socket_;
  std::string url_;
  WebSocketListener* listener_;
};

#endif  // DONG_SHANG_CHROME_BROWSER_WEBSOCKET_CLIENT_H_
