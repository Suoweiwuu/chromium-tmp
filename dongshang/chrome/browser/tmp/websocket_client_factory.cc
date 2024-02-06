#include "content/public/browser/dongshang/websocket_client_factory.h"
#include "content/public/browser/dongshang/message_received_listener.h"




WebsocketClientFactory::WebsocketClientFactory() {}

WebsocketClientFactory::~WebsocketClientFactory() {}

std::shared_ptr<WebsocketClient> WebsocketClientFactory::GetInstance(
    std::string url,
    WebSocketListener* listener) {
  return std::make_shared<WebsocketClient>(url, listener);
}

std::shared_ptr<WebsocketClient> WebsocketClientFactory::GetOrCreateDefaultInstance() {
  if (!default_instance_) {
    default_instance_ =
        GetInstance("ws://127.0.0.1:8989", new MessageReceivedListener());
  }
  return default_instance_;
}
