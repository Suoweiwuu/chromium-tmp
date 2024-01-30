#ifndef DONG_SHANG_CHROME_BROWSER_MESSAGE_RECEIVED_LISTENER_H_
#define DONG_SHANG_CHROME_BROWSER_MESSAGE_RECEIVED_LISTENER_H_

#include "dongshang/chrome/browser/websocket_listener.h"

class MessageReceivedListener : public WebSocketListener {
 public:
  MessageReceivedListener();
  ~MessageReceivedListener() override;

  void OnMessageReceived(const std::string& message) override;

  void OnClose() override;
};

#endif  // DONG_SHANG_CHROME_BROWSER_MESSAGE_RECEIVED_LISTENER_H_
