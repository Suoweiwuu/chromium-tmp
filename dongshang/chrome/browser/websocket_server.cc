#include "dongshang/chrome/browser/websocket_server.h"

#include "base/command_line.h"
#include "base/strings/string_number_conversions.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/net/system_network_context_manager.h"

namespace {
const net::NetworkTrafficAnnotationTag kServerTag =
    net::DefineNetworkTrafficAnnotation("ui_devtools_server", R"()");

const char kWebsocketPort[] = "ds-websocket-port";
}  // namespace

WebSocketServer::WebSocketServer(WebSocketServerDelegate* delegate)
    : delegate_(delegate), port_(9002), tag_(kServerTag) {}

WebSocketServer ::~WebSocketServer() {}

bool WebSocketServer::Init() {
  base::CommandLine* cmd = base::CommandLine::ForCurrentProcess();
  if (cmd->HasSwitch(kWebsocketPort)) {
    std::string port = cmd->GetSwitchValueASCII(kWebsocketPort);
    if (port.empty()) {
      LOG(WARNING) << "--ds-websocket-port is empty";
      return false;
    }

    if (!base::StringToInt(port, &port_)) {
      LOG(WARNING) << "--ds-websocket-port port is valid";
      return false;
    }
  } else {
    LOG(WARNING) << "without --ds-websocket-port commandline";
  }

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
  LOG(WARNING) << "WebSocketServer::Shutdown";
  server_.reset();
  return true;
}

void WebSocketServer::SendOverWebSocket(int connection_id,
                                        base::StringPiece message) {
  if (server_) {
    LOG(WARNING) << "WebSocketServer::SendOverWebSocket " << connection_id;
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
  LOG(WARNING) << "WebSocketServer port is " << port;

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
  LOG(WARNING) << "MakeServer result is " << result;
  if (result == net::OK) {
    server_ = std::make_unique<network::server::HttpServer>(
        std::move(server_socket), this);
  } else {
    LOG(WARNING) << "MakeServer failed " << result;
  }
}

// HttpServer::Delegate Implementation
void WebSocketServer::OnConnect(int connection_id) {
  LOG(WARNING) << "WebSocketServer::OnConnect " << connection_id;
  if (delegate_) {
    delegate_->OnConnect(connection_id);
  }
}

void WebSocketServer::OnHttpRequest(
    int connection_id,
    const network::server::HttpServerRequestInfo& info) {
  LOG(WARNING) << "WebSocketServer::OnHttpRequest " << connection_id;
  NOTIMPLEMENTED();
}

void WebSocketServer::OnWebSocketRequest(
    int connection_id,
    const network::server::HttpServerRequestInfo& info) {
  LOG(WARNING) << "WebSocketServer::OnWebSocketRequest " << connection_id;
  if (server_) {
    server_->AcceptWebSocket(connection_id, info, tag_);
  }
}

void WebSocketServer::OnWebSocketMessage(int connection_id, std::string data) {
  LOG(WARNING) << "WebSocketServer::OnWebSocketMessage " << connection_id;
  LOG(WARNING) << data;
  if (delegate_) {
    delegate_->OnWebSocketMessage(connection_id, data);
  }
}

void WebSocketServer::OnClose(int connection_id) {
  LOG(WARNING) << "WebSocketServer::OnClose " << connection_id;
  if (delegate_) {
    delegate_->OnClose(connection_id);
  }
}