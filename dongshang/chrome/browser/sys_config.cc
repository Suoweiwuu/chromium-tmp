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
#include "base/values.h"


#include "dongshang/chrome/browser/sys_config.h"


SysConfig::SysConfig() {
  base::FilePath exe_path =
      base::CommandLine::ForCurrentProcess()->GetProgram();
  base::FilePath sys_config_path =
      exe_path.DirName().Append(FILE_PATH_LITERAL("sys_config.json"));

  std::string confgi_str = base::ReadStrFromFile(sys_config_path.AsUTF8Unsafe());

  absl::optional<base::Value> config = base::JSONReader::Read(base::StringPiece(config_str));

  base::Value* source_code_path = config->FindDictKey("source_code_path");

  if (source_code_path) {
    source_code_path_ = source_code_path->GetString();
  }

  base::Value* git_repo_url = config->FindDictKey("git_repo_url");
  if (git_repo_url) {
    git_repo_url_ = git_repo_url->GetString();
  }

}
SysConfig::~SysConfig() {}


SysConfig* SysConfig::GetInstance() {
  return base::Singleton<SysConfig>::get();
}

