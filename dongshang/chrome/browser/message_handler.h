#ifndef DONG_SHANG_CHROME_BROWSER_MESSAGE_HANDLER_H_
#define DONG_SHANG_CHROME_BROWSER_MESSAGE_HANDLER_H_

#include <memory>

#include "base/memory/weak_ptr.h"
#include "base/values.h"
#include "dongshang/chrome/browser/websocket_server_delegate.h"
#include "third_party/blink/public/common/web_preferences/web_preferences.h"
#include "third_party/blink/public/common/widget/device_emulation_params.h"
#include "ui/gfx/image/image.h"

class WebSocketServer;

class MessageHandler : WebSocketServerDelegate {
 public:
  MessageHandler();
  ~MessageHandler() override;

  MessageHandler(const MessageHandler&) = delete;
  MessageHandler& operator=(const MessageHandler&) = delete;

  void StartWebSocketServer();
  void StopWebSocketServer();

  void OnConnect(int connection_id) override;
  void OnWebSocketMessage(int connection_id, std::string data) override;
  void OnClose(int connection_id) override;

 private:
  void GetActiveTabId(int connection_id, const base::Value::Dict* dict);
  void GetWindowIdForTabId(int connection_id, const base::Value::Dict* dict);
  void GetFrameIndex(int connection_id, const base::Value::Dict* dict);
  void GetHtmlValue(int connection_id, const base::Value::Dict* dict);
  void CaptureHtmlElement(int connection_id, const base::Value::Dict* dict);

  void SendMessage(int connection_id, const base::Value::Dict* dict);
  void OnGetInnerHtml(int connection_id,
                      std::string request_id,
                      base::Value result);
  void OnGetElementRect(int connection_id,
                        std::string request_id,
                        base::Value result);
  void ScreenshotCaptured(
      int connection_id,
      std::string request_id,
      const gfx::Size& original_view_size,
      const gfx::Size& requested_image_size,
      const blink::DeviceEmulationParams& original_params,
      const absl::optional<blink::web_pref::WebPreferences>& original_web_prefs,
      const gfx::Image& image);

  std::unique_ptr<WebSocketServer> websocket_server_;
  base::WeakPtrFactory<MessageHandler> weak_factory_{this};
};

#endif  // DONG_SHANG_CHROME_BROWSER_MESSAGE_HANDLER_H_
