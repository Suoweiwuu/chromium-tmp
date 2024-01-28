#include "content/public/browser/dongshang/websocket_server.h"

#include "chrome/browser/browser_process.h"
#include "chrome/browser/net/system_network_context_manager.h"

namespace {
const net::NetworkTrafficAnnotationTag kServerTag =
    net::DefineNetworkTrafficAnnotation("ui_devtools_server", R"()");
}

WebSocketServer::WebSocketServer() : port_(9002), tag_(kServerTag) {}

WebSocketServer ::~WebSocketServer() {}

bool WebSocketServer::Init() {
  mojo::PendingRemote<network::mojom::TCPServerSocket> server_socket;
  auto receiver = server_socket.InitWithNewPipeAndPassReceiver();
  CreateTCPServerSocket(
      std::move(receiver),
      g_browser_process->system_network_context_manager()->GetContext(), port_,
      tag_,
      base::BindOnce(&WebSocketServer::MakeServer, weak_factory_.GetWeakPtr(),
                     std::move(server_socket)));

  return true;
}

bool WebSocketServer::Shutdown() {
  server_.reset();
  return true;
}

void WebSocketServer::SendOverWebSocket(int connection_id,
                                        base::StringPiece message) {
  if (server_) {
    server_->SendOverWebSocket(connection_id, message, tag_);
  }
}

void WebSocketServer::CreateTCPServerSocket(
    mojo::PendingReceiver<network::mojom::TCPServerSocket>
        server_socket_receiver,
    network::mojom::NetworkContext* network_context,
    int port,
    net::NetworkTrafficAnnotationTag tag,
    network::mojom::NetworkContext::CreateTCPServerSocketCallback callback) {
  // Create the socket using the address 127.0.0.1 to listen on all interfaces.
  net::IPAddress address(127, 0, 0, 1);
  constexpr int kBacklog = 1;
  network_context->CreateTCPServerSocket(
      net::IPEndPoint(address, port), kBacklog,
      net::MutableNetworkTrafficAnnotationTag(tag),
      std::move(server_socket_receiver), std::move(callback));
}

void WebSocketServer::MakeServer(
    mojo::PendingRemote<network::mojom::TCPServerSocket> server_socket,
    int result,
    const absl::optional<net::IPEndPoint>& local_addr) {
  if (result == net::OK) {
    server_ = std::make_unique<network::server::HttpServer>(
        std::move(server_socket), this);
  }
}

// HttpServer::Delegate Implementation
void WebSocketServer::OnConnect(int connection_id) {
  int ia = 0;
  ia++;
}

void WebSocketServer::OnHttpRequest(
    int connection_id,
    const network::server::HttpServerRequestInfo& info) {
  NOTIMPLEMENTED();
}

void WebSocketServer::OnWebSocketRequest(
    int connection_id,
    const network::server::HttpServerRequestInfo& info) {
  if (server_) {
    server_->AcceptWebSocket(connection_id, info, tag_);
  }
}

void WebSocketServer::OnWebSocketMessage(int connection_id, std::string data) {
  int ia = 0;
  ia++;
}

void WebSocketServer::OnClose(int connection_id) {
  int ia = 0;
  ia++;
}