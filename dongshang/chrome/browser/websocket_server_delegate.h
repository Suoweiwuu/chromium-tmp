#ifndef DONG_SHANG_CHROME_BROWSER_WEBSOCKET_SERVER_DELEGATE_H_
#define DONG_SHANG_CHROME_BROWSER_WEBSOCKET_SERVER_DELEGATE_H_

#include <string>

class WebSocketServerDelegate {
 public:
  WebSocketServerDelegate() {}
  virtual ~WebSocketServerDelegate() {}

  virtual void OnConnect(int connection_id) {}
  virtual void OnWebSocketMessage(int connection_id, std::string data) {}
  virtual void OnClose(int connection_id) {}
};

#endif  // DONG_SHANG_CHROME_BROWSER_WEBSOCKET_SERVER_DELEGATE_H_
