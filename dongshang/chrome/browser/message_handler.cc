#include "dongshang/chrome/browser/message_handler.h"

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
  int ia = 0;
  ia++;
}

void MessageHandler::OnClose(int connection_id) {
  int ia = 0;
  ia++;
}