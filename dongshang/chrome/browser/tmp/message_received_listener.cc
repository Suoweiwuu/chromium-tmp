#include "content/public/browser/dongshang/message_received_listener.h"

#include "base/logging.h"
#include "base/values.h"

#include "content/public/browser/render_frame_host.h"
#include "content/browser/renderer_host/render_frame_host_impl.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "content/public/browser/web_contents.h"


MessageReceivedListener::MessageReceivedListener() {}
MessageReceivedListener::~MessageReceivedListener() {}


void ExecCallback(base::Value value) {
  LOG(INFO) << value;
}

void MessageReceivedListener::OnMessageReceived(
    const std::string& message) {
  LOG(INFO) << message;
 /* Browser* browser = chrome::FindLastActive();

  content::RenderFrameHost* const frame = browser->tab_strip_model()->GetActiveWebContents()->GetMainFrame();
  frame->ExecuteJavaScript("", base::BindOnce(&ExecCallback));*/
}

void MessageReceivedListener::OnClose() {
  LOG(INFO) << "OnClose";
}