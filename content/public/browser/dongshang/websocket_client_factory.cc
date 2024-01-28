#define MY_BASE_IMPLEMENTATION

#include "content/public/browser/dongshang/websocket_client_factory.h"

MY_EXPORT std::shared_ptr<WebsocketClient>
    WebsocketClientFactory::default_instance_;

//WebsocketClientFactory::WebsocketClientFactory() {}
//
//WebsocketClientFactory::~WebsocketClientFactory() {}

MY_EXPORT inline std::shared_ptr<WebsocketClient>
WebsocketClientFactory::GetInstance(
    std::string url,
    WebSocketListener* listener) {
  return std::make_shared<WebsocketClient>(url, listener);
}

MY_EXPORT inline std::shared_ptr<WebsocketClient>
WebsocketClientFactory::CreateAndGetDefaultInstance(
    std::string url,
    WebSocketListener* listener) {
  default_instance_ = std::make_shared<WebsocketClient>(url, listener);
  return default_instance_;
}

std::shared_ptr<WebsocketClient>
WebsocketClientFactory::GetDefaultInstance() {
  return default_instance_;
}

