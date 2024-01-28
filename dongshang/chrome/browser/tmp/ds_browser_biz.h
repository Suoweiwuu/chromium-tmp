#ifndef DONG_SHANG_CHROME_BROWSER_DS_BROWSER_BIZ_H_
#define DONG_SHANG_CHROME_BROWSER_DS_BROWSER_BIZ_H_

#include <memory>

#include "base/memory/weak_ptr.h"

#include "dongshang/chrome/browser/websocket_client.h"

class DsBrowserBiz {
 public:
  DsBrowserBiz();
  ~DsBrowserBiz();

  DsBrowserBiz(const DsBrowserBiz&) = delete;
  DsBrowserBiz& operator=(const DsBrowserBiz&) = delete;

  void PreMainMessageLoopRun();
  void PostBrowserStart();

 private:
  std::shared_ptr<WebsocketClient> websocket_client_;

  //std::unique_ptr<WebSocketServer> websocket_server_;
  base::WeakPtrFactory<DsBrowserBiz> weak_factory_{this};
};

#endif  // DONG_SHANG_CHROME_BROWSER_DS_BROWSER_BIZ_H_
