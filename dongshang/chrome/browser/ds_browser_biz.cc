#include "dongshang/chrome/browser/ds_browser_biz.h"

#include "dongshang/chrome/browser/extension_installer.h"
#include "dongshang/chrome/browser/websocket_server.h"

DsBrowserBiz::DsBrowserBiz() {}

DsBrowserBiz::~DsBrowserBiz() {}

void DsBrowserBiz::PreMainMessageLoopRun() {}

void DsBrowserBiz::PostBrowserStart() {
  websocket_server_.reset(new WebSocketServer);
  websocket_server_->Init();

  //InstallDefaultExtension();
}