#include "content/public/browser/dongshang/websocket_client.h"
#include "content/public/browser/dongshang/websocket.h"

#include <stddef.h>

#include <memory>
#include <string>
#include <vector>

#include "base/base64.h"
#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/location.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/ref_counted.h"
#include "base/run_loop.h"
#include "base/threading/thread.h"
#include "base/time/time.h"
#include "url/gurl.h"


void OnConnectFinished(base::RunLoop* run_loop, int* save_error, int error) {
  *save_error = error;
  run_loop->Quit();
}

WebsocketClient::WebsocketClient(std::string url, WebSocketListener* listener) {
  url_ = url;
  listener_ = listener;
}

WebsocketClient::~WebsocketClient() {}


bool WebsocketClient::CreateWebSocket() {
    int error;
    std::unique_ptr<WebSocket> sock(new WebSocket(GURL(url_), listener_));
    base::RunLoop run_loop;
    sock->Connect(base::BindOnce(&OnConnectFinished, &run_loop, &error));

    run_loop.Run();
    if (error == net::OK) {
      socket_ = std::move(sock);
      return true;  
    }
    return false;
}

bool WebsocketClient::Connect() {

  return CreateWebSocket();
}


bool WebsocketClient::Disconnect() {
  LOG(INFO) << "Disconnect";
  return true;
}

void WebsocketClient::Send(std::string message) {
  socket_->Send(message);
}