// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_DONG_SHANG_WEBSOCKET_LISTENER_H_
#define CONTENT_PUBLIC_BROWSER_DONG_SHANG_WEBSOCKET_LISTENER_H_

#include <string>

// Listens for WebSocket messages and disconnects on the same thread as the
// WebSocket.
class WebSocketListener {
 public:
  virtual ~WebSocketListener() {}

  // Called when a WebSocket message is received.
  virtual void OnMessageReceived(const std::string& message) = 0;

  // Called when the WebSocket connection closes. Will be called at most once.
  // Will not be called if the connection was never established or if the close
  // was initiated by the client.
  virtual void OnClose() = 0;
};

#endif  // CONTENT_PUBLIC_BROWSER_DONG_SHANG_WEBSOCKET_LISTENER_H_
