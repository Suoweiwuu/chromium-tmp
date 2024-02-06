#include "dongshang/chrome/browser/ds_browser_biz.h"

#include "dongshang/chrome/browser/websocket_client_factory.h"

DsBrowserBiz::DsBrowserBiz() {}

DsBrowserBiz::~DsBrowserBiz() {}

void DsBrowserBiz::PreMainMessageLoopRun() {}

void handle_message(const std::string& message) {
  printf(">>> %s\n", message.c_str());
}

void DsBrowserBiz::PostBrowserStart() {
  websocket_client_ = WebsocketClientFactory::GetOrCreateDefaultInstance();
  websocket_client_->Connect();
}