#ifndef DONG_SHANG_CHROME_BROWSER_MESSAGE_RECEIVED_LISTENER_H_
#define DONG_SHANG_CHROME_BROWSER_MESSAGE_RECEIVED_LISTENER_H_

#include "base/values.h"
#include "content/public/browser/dongshang/websocket_listener.h"
#include "content/public/browser/dongshang/websocket_client.h"



class MessageReceivedListener : public WebSocketListener {
 public:
  MessageReceivedListener();
  ~MessageReceivedListener() override;

  void ExecCallback(base::Value value);

  void OnMessageReceived(const std::string& message) override;

  void OnClose() override;

  void SetClient(std::shared_ptr<WebsocketClient> client);

 private:
  std::shared_ptr<WebsocketClient> client_;
};

#endif  // DONG_SHANG_CHROME_BROWSER_MESSAGE_RECEIVED_LISTENER_H_
