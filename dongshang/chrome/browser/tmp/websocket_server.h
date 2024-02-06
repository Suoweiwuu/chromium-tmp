#ifndef DONG_SHANG_CHROME_BROWSER_WEBSOCKET_SERVER_H_
#define DONG_SHANG_CHROME_BROWSER_WEBSOCKET_SERVER_H_

#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "services/network/public/cpp/server/http_server.h"
#include "services/network/public/mojom/network_context.mojom.h"

// https://www.piesocket.com/websocket-tester

class WebSocketServer : public network::server::HttpServer::Delegate {
 public:
  WebSocketServer();
  ~WebSocketServer() override;

  bool Init();
  bool Shutdown();

  // HttpServer::Delegate
  void OnConnect(int connection_id) override;
  void OnHttpRequest(
      int connection_id,
      const network::server::HttpServerRequestInfo& info) override;
  void OnWebSocketRequest(
      int connection_id,
      const network::server::HttpServerRequestInfo& info) override;
  void OnWebSocketMessage(int connection_id, std::string data) override;
  void OnClose(int connection_id) override;

 private:
  WebSocketServer(const WebSocketServer&) = delete;
  WebSocketServer& operator=(const WebSocketServer&) = delete;

  // Creates a TCPServerSocket to be used by a UiDevToolsServer.
  static void CreateTCPServerSocket(
      mojo::PendingReceiver<network::mojom::TCPServerSocket>
          server_socket_receiver,
      network::mojom::NetworkContext* network_context,
      int port,
      net::NetworkTrafficAnnotationTag tag,
      network::mojom::NetworkContext::CreateTCPServerSocketCallback callback);

  void MakeServer(
      mojo::PendingRemote<network::mojom::TCPServerSocket> server_socket,
      int result,
      const absl::optional<net::IPEndPoint>& local_addr);

  std::unique_ptr<network::server::HttpServer> server_;

  // The port the devtools server listens on
  int port_;

  const net::NetworkTrafficAnnotationTag tag_;

  base::WeakPtrFactory<WebSocketServer> weak_factory_{this};
};

#endif  // DONG_SHANG_CHROME_BROWSER_WEBSOCKET_SERVER_H_
