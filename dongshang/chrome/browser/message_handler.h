#ifndef DONG_SHANG_CHROME_BROWSER_MESSAGE_HANDLER_H_
#define DONG_SHANG_CHROME_BROWSER_MESSAGE_HANDLER_H_

#include <memory>

#include "base/memory/weak_ptr.h"

class WebSocketServer;

class MessageHandler {
 public:
  MessageHandler();
  ~MessageHandler();

  MessageHandler(const MessageHandler&) = delete;
  MessageHandler& operator=(const MessageHandler&) = delete;

  void StartWebSocketServer();
  void StopWebSocketServer();

 private:
  std::unique_ptr<WebSocketServer> websocket_server_;
  base::WeakPtrFactory<MessageHandler> weak_factory_{this};
};

#endif  // DONG_SHANG_CHROME_BROWSER_MESSAGE_HANDLER_H_
