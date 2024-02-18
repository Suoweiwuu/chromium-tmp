
#include "dongshang/chrome/browser/ds_browser_biz.h"
#include "dongshang/chrome/browser/message_received_listener.h"
#include "dongshang/chrome/browser/script_proxy.h"

#include "base/task/post_task.h"
#include "base/run_loop.h"
#include "base/task/single_thread_task_executor.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "content/public/browser/dongshang/websocket_client_factory.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/browser_task_traits.h"

DsBrowserBiz::DsBrowserBiz() {}

DsBrowserBiz::~DsBrowserBiz() {}

void DsBrowserBiz::PreMainMessageLoopRun() {}

void handle_message(const std::string& message) {
  printf(">>> %s\n", message.c_str());
}


void DsBrowserBiz::StartConnectWebsocket() {
  MessageReceivedListener* listener = new MessageReceivedListener();
  websocket_client_ = WebsocketClientFactory::CreateAndGetDefaultInstance(
      "ws://192.168.121.185:8989", listener);
  listener->SetClient(websocket_client_);
  websocket_client_->Connect();
}


void DsBrowserBiz::PostBrowserStart() {
  //__debugbreak();
  //StartConnectWebsocket();
  //base::ThreadPoolInstance::CreateAndStartWithDefaultParams("WebSocketConnector");

  content::GetIOThreadTaskRunner({})->PostTask(FROM_HERE, base::BindOnce(&DsBrowserBiz::StartConnectWebsocket, base::Unretained(this)));

  //scoped_refptr<base::SingleThreadTaskRunner> task_runner_ =
  //    base::CreateSingleThreadTaskRunner(
  //        {base::TaskShutdownBehavior::BLOCK_SHUTDOWN},

          //base::SingleThreadTaskRunnerThreadMode::DEDICATED);
  // Start the WebSocket server in a separate thread
  //base::ThreadPoolInstance::Get()->PostTask(
  //    FROM_HERE, base::BindOnce(&StartConnectWebsocket));
  /*base::PostTask(
      FROM_HERE, {base::TaskShutdownBehavior::BLOCK_SHUTDOWN},
      base::BindOnce(&DsBrowserBiz::StartConnectWebsocket,
                     base::Unretained(this)));

  base::RunLoop().Run();*/
}