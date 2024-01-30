
#include "dongshang/chrome/browser/ds_browser_biz.h"
#include "dongshang/chrome/browser/message_received_listener.h"
#include "dongshang/chrome/browser/script_proxy.h"

#include "content/public/browser/dongshang/websocket_client_factory.h"
#include "content/public/browser/web_contents.h"

DsBrowserBiz::DsBrowserBiz() {}

DsBrowserBiz::~DsBrowserBiz() {}

void DsBrowserBiz::PreMainMessageLoopRun() {}

void handle_message(const std::string& message) {
  printf(">>> %s\n", message.c_str());
}

void DsBrowserBiz::PostBrowserStart() {
  MessageReceivedListener* listener = new MessageReceivedListener();
  websocket_client_ = WebsocketClientFactory::CreateAndGetDefaultInstance(
      "ws://127.0.0.1:8989", listener);
  listener->SetClient(websocket_client_);
  websocket_client_->Connect();

}