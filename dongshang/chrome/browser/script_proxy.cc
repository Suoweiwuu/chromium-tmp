#include <fstream>
#include <map>
#include <vector>

#include "content/public/browser/render_frame_host.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/browser/ui/browser_finder.h"
#include "content/public/browser/web_contents.h"

#include <fstream>
#include <map>
#include <string>
#include <vector>
#include "base/base64.h"
#include "base/bind.h"
#include "base/files/file_util.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/memory/singleton.h"


#include "dongshang/chrome/browser/script_proxy.h"
#include "base/memory/ptr_util.h"

#include "content/public/common/referrer.h"


ScriptProxy::ScriptProxy() {}
ScriptProxy::~ScriptProxy() {}


bool ScriptProxy::ExecuteJsCode(std::string code) {
   Browser* browser = chrome::FindLastActive();
   content::RenderFrameHost* const render_frame_host =
       browser->tab_strip_model()->GetActiveWebContents()->GetMainFrame();
   render_frame_host->AllowInjectingJavaScript();
   render_frame_host->ExecuteJavaScript(base::UTF8ToUTF16(code),
       base::BindOnce(&ScriptProxy::ExecuteJsCodeCallback,
                      weak_factory_.GetWeakPtr()));
   return false;
}

void ScriptProxy::ExecuteJsCodeCallback(base::Value result) {
  LOG(INFO) << result;
}

ScriptProxy* ScriptProxy::GetInstance() {
   return base::Singleton<ScriptProxy>::get();
}

