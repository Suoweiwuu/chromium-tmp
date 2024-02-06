#ifndef DONG_SHANG_CHROME_BROWSER_WEBSOCKET_CLIENT_FACTORY_H_
#define DONG_SHANG_CHROME_BROWSER_WEBSOCKET_CLIENT_FACTORY_H_

#include <string>
#include "dongshang/chrome/browser/websocket.h"
#include "dongshang/chrome/browser/websocket_listener.h"
#include "dongshang/chrome/browser/websocket_client.h"


class WebsocketClientFactory {
 public:
  WebsocketClientFactory();
  ~WebsocketClientFactory();

  WebsocketClientFactory(const WebsocketClientFactory&) = delete;
  WebsocketClientFactory& operator=(const WebsocketClientFactory&) = delete;

  static std::shared_ptr<WebsocketClient> GetInstance(
      std::string url, WebSocketListener* listener);

  static std::shared_ptr<WebsocketClient> GetOrCreateDefaultInstance();

 private:
  static std::shared_ptr<WebsocketClient> default_instance_;
};

#endif  // DONG_SHANG_CHROME_BROWSER_WEBSOCKET_CLIENT_FACTORY_H_
