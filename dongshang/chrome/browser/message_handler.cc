#include "dongshang/chrome/browser/message_handler.h"

#include "dongshang/chrome/browser/websocket_server.h"

MessageHandler::MessageHandler() {}

MessageHandler::~MessageHandler() {}

void MessageHandler::StartWebSocketServer() {
  websocket_server_.reset(new WebSocketServer);
  websocket_server_->Init();
}

void MessageHandler::StopWebSocketServer() {
  if (websocket_server_) {
    websocket_server_.reset();
  }
}