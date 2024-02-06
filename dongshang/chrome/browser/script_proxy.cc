#include <fstream>
#include <map>
#include <string>
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

//
//std::string ReadJsCode() {
//    std::string directory = "C:/Users/wudi/Documents/nice-assistant";
//    std::vector<std::string> fileNames;
//    std::string fullCode;
//    FileUtils::GetAllFiles(directory, fileNames);
//    for (std::vector<std::string>::iterator it = fileNames.begin(); it != fileNames.end();it++) {
//        std::string code = FileUtils::ReadStrFromFile(*it);
//        fullCode += code + "\n";
//    }
//    return fullCode;
//}
//
//bool ScriptProxy::InitializeJsEnv() {
//   Browser* browser = chrome::FindLastActive();
//
//   //content::RenderFrameHost* const render_frame_host = browser->tab_strip_model()->GetActiveWebContents()->GetMainFrame();
//
//
//   content::OpenURLParams params(GURL("https://www.baidu.com"), content::Referrer(),
//                                 WindowOpenDisposition::CURRENT_TAB,
//                                 ui::PAGE_TRANSITION_LINK, false);
//   content::WebContents* web_content = browser->tab_strip_model()->GetActiveWebContents()->OpenURL(params);
//   content::RenderFrameHost* const render_frame_host = web_content->GetMainFrame();
//
//
//   //ExecuteJavaScript(base::UTF8ToUTF16(code));
//    
//   std::string code = ReadJsCode();
//
//   code = "window.TestFunc = function() {console.log(1)}";
//
//   LOG(INFO) << "code -> " <<  code;
//
//   base::WriteFile(base::FilePath(FILE_PATH_LITERAL("C:/tmp/a.txt")), code);
//
//   render_frame_host->AllowInjectingJavaScript();
//   render_frame_host->ExecuteJavaScript(
//       base::UTF8ToUTF16(code),
//       base::BindOnce(&ScriptProxy::InitializeJsEnvCallback,
//                      weak_factory_.GetWeakPtr()));
//   return true;
//}

//void ScriptProxy::InitializeJsEnvCallback(base::Value result) {
//  LOG(INFO) << "success";
//}

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

