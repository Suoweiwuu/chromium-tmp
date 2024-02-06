// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_GIN_CPP_BRIDGE_MESSAGES_H_
#define CONTENT_COMMON_GIN_CPP_BRIDGE_MESSAGES_H_

// IPC messages for injected C++ objects (Gin-based implementation).

#include <stdint.h>

#include "content/common/content_export.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_message_start.h"

#undef IPC_MESSAGE_EXPORT
#define IPC_MESSAGE_EXPORT CONTENT_EXPORT
#define IPC_MESSAGE_START GinCppBridgeMsgStart


//callback js function result to browser
IPC_MESSAGE_ROUTED1(GinCppBridgeHostMsg_CallbackResult,
                    std::string /* message */)

#endif  // CONTENT_COMMON_GIN_CPP_BRIDGE_MESSAGES_H_
