#include "dongshang/chrome/browser/ds_browser_biz.h"

#include "dongshang/chrome/browser/extension_installer.h"
#include "dongshang/chrome/browser/message_handler.h"

DsBrowserBiz::DsBrowserBiz() {}

DsBrowserBiz::~DsBrowserBiz() {}

void DsBrowserBiz::PreMainMessageLoopRun() {}

void DsBrowserBiz::PostBrowserStart() {
  message_handler_.reset(new MessageHandler);
  message_handler_->StartWebSocketServer();

  // InstallDefaultExtension();
}

void DsBrowserBiz::PostMainMessageLoopRun() {
  if (message_handler_) {
    message_handler_->StopWebSocketServer();
    message_handler_.reset();
  }
}