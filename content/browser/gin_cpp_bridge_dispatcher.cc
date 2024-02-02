// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/gin_cpp_bridge_dispatcher.h"
    
#include "content/common/gin_cpp_bridge_messages.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/dongshang/websocket_client_factory.h"

namespace content {

GinCppBridgeDispatcher::GinCppBridgeDispatcher(
    WebContentsImpl* web_contents)
    : WebContentsObserver(web_contents) {}

GinCppBridgeDispatcher::~GinCppBridgeDispatcher() {}

void GinCppBridgeDispatcher::OnUploadData(std::string message) {
  LOG(INFO) << "Upload message => " << message.c_str();

  std::shared_ptr<WebsocketClient> websocket_client = WebsocketClientFactory::GetDefaultInstance();
  if (websocket_client)
    websocket_client->Send(message);
  LOG(INFO) << "onUploadData Success ";
}

bool GinCppBridgeDispatcher::OnMessageReceived(
    const IPC::Message& message,
    RenderFrameHost* render_frame_host) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(GinCppBridgeDispatcher, message)
    IPC_MESSAGE_HANDLER(GinCppBridgeHostMsg_CallbackResult, OnUploadData)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}
}  // namespace content
