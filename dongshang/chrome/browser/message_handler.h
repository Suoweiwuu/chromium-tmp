#ifndef DONG_SHANG_CHROME_BROWSER_MESSAGE_HANDLER_H_
#define DONG_SHANG_CHROME_BROWSER_MESSAGE_HANDLER_H_

#include <memory>

#include "base/memory/weak_ptr.h"
#include "dongshang/chrome/browser/websocket_server_delegate.h"

class WebSocketServer;

class MessageHandler : WebSocketServerDelegate {
 public:
  MessageHandler();
  ~MessageHandler() override;

  MessageHandler(const MessageHandler&) = delete;
  MessageHandler& operator=(const MessageHandler&) = delete;

  void StartWebSocketServer();
  void StopWebSocketServer();

  void OnConnect(int connection_id) override;
  void OnWebSocketMessage(int connection_id, std::string data) override;
  void OnClose(int connection_id) override;

 private:
  std::unique_ptr<WebSocketServer> websocket_server_;
  base::WeakPtrFactory<MessageHandler> weak_factory_{this};
};

#endif  // DONG_SHANG_CHROME_BROWSER_MESSAGE_HANDLER_H_
