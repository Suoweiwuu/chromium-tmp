#include "dongshang/chrome/browser/message_handler.h"

#include "base/json/json_reader.h"
#include "dongshang/chrome/browser/websocket_server.h"

MessageHandler::MessageHandler() {}

MessageHandler::~MessageHandler() {}

void MessageHandler::StartWebSocketServer() {
  websocket_server_.reset(new WebSocketServer(this));
  websocket_server_->Init();
}

void MessageHandler::StopWebSocketServer() {
  if (websocket_server_) {
    websocket_server_.reset();
  }
}

void MessageHandler::OnConnect(int connection_id) {
  int ia = 0;
  ia++;
}

void MessageHandler::OnWebSocketMessage(int connection_id, std::string data) {
  absl::optional<base::Value> value = base::JSONReader::Read(data);
  if (!value) {
    return;
  }

  const base::Value::Dict* dict = value->GetIfDict();
  if (!dict) {
    return;
  }

  const std::string* command_name = dict->FindString("commandName");
  if (!command_name) {
    return;
  }

  if (*command_name == std::string("GetActiveTabId")) {
    GetActiveTabId(connection_id, dict);
  } else if (*command_name == std::string("GetWindowIdForTabId")) {
    GetWindowIdForTabId(connection_id, dict);
  } else if (*command_name == std::string("GetFrameIndex")) {
    GetFrameIndex(connection_id, dict);
  } else if (*command_name == std::string("GetHtmlValue")) {
    GetHtmlValue(connection_id, dict);
  } else {
    LOG(ERROR) << "unknown command name:" << command_name->c_str();
  }
}

void MessageHandler::OnClose(int connection_id) {
  int ia = 0;
  ia++;
}

void MessageHandler::GetActiveTabId(int connection_id,
                                    const base::Value::Dict* dict) {}

void MessageHandler::GetWindowIdForTabId(int connection_id,
                                         const base::Value::Dict* dict) {}

void MessageHandler::GetFrameIndex(int connection_id,
                                   const base::Value::Dict* dict) {}

void MessageHandler::GetHtmlValue(int connection_id,
                                  const base::Value::Dict* dict) {}