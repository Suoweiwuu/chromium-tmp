// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_GIN_CPP_BRIDGE_DISPATCHER_H_
#define CONTENT_BROWSER_GIN_CPP_BRIDGE_DISPATCHER_H_

#include <map>
#include <memory>
#include <set>

#include "base/containers/id_map.h"
#include "base/memory/weak_ptr.h"
#include "base/values.h"
#include "content/public/renderer/render_frame_observer.h"
#include "content/browser/web_contents/web_contents_impl.h"

namespace content {


class GinCppBridgeDispatcher : public WebContentsObserver {
 public:
  explicit GinCppBridgeDispatcher(WebContentsImpl* web_contents);

  GinCppBridgeDispatcher(const GinCppBridgeDispatcher&) = delete;
  GinCppBridgeDispatcher& operator=(const GinCppBridgeDispatcher&) = delete;

  ~GinCppBridgeDispatcher() override;

  void OnUploadData(std::string message);
  // RenderFrameObserver override:
  bool OnMessageReceived(const IPC::Message& message,
                         RenderFrameHost* render_frame_host) override;
 
};

}  // namespace content

#endif  // CONTENT_RENDERER_CPP_GIN_JAVA_BRIDGE_DISPATCHER_H_
