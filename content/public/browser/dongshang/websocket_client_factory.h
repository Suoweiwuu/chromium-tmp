#ifndef CONTENT_PUBLIC_BROWSER_DONG_SHANG_WEBSOCKET_CLIENT_FACTORY_H_
#define CONTENT_PUBLIC_BROWSER_DONG_SHANG_WEBSOCKET_CLIENT_FACTORY_H_


#include <string>
#include "content/public/browser/dongshang/websocket_client.h"


//BASE_EXPORT extern std::shared_ptr<WebsocketClient> default_instance_;

class  WebsocketClientFactory {
 public:
  WebsocketClientFactory();
  ~WebsocketClientFactory();

  WebsocketClientFactory(const WebsocketClientFactory&) = delete;
  WebsocketClientFactory& operator=(const WebsocketClientFactory&) = delete;

  MY_EXPORT static std::shared_ptr<WebsocketClient> GetInstance(
      std::string url, WebSocketListener* listener);

  MY_EXPORT static std::shared_ptr<WebsocketClient> CreateAndGetDefaultInstance(
      std::string url,
      WebSocketListener* listener);

  static std::shared_ptr<WebsocketClient> GetDefaultInstance();

 private:
  MY_EXPORT static std::shared_ptr<WebsocketClient> default_instance_;
};

#endif  // CONTENT_PUBLIC_BROWSER_DONG_SHANG_WEBSOCKET_CLIENT_FACTORY_H__
