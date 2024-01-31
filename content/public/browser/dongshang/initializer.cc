#define MY_BASE_IMPLEMENTATION

#include <string>
#include <vector>
#include "content/public/browser/dongshang/initializer.h"
#include "base/files/file_util.h"
#include "base/files/file_enumerator.h"
#include "base/memory/singleton.h"
#include "base/logging.h"
#include "base/command_line.h"


void Initializer::InitJsCode() {
//#if BUILDFLAG(IS_WIN)
//  std::string directory(u8"./nice-assistant");
//#else
//  std::string directory(u8"/home/yiwise/chromium2/src/out/Default/nice-assistant");
//#endif

  base::FilePath exe_path = base::CommandLine::ForCurrentProcess()->GetProgram();
  base::FilePath jsCodePath = exe_path.DirName().Append(FILE_PATH_LITERAL("nice-assistant"));
  LOG(INFO) << "exe_path:" << jsCodePath.AsUTF8Unsafe();

  std::string directory = jsCodePath.AsUTF8Unsafe();
  std::vector<std::string> fileNames;
  std::string full_code;
  base::GetAllFiles(directory, fileNames);

  for (std::vector<std::string>::iterator it = fileNames.begin();
       it != fileNames.end(); it++) {
    std::string code = base::ReadStrFromFile(*it);
    full_code += code + "\n";
  }
  js_code_ = full_code;
  LOG(INFO) << "InitJsCode:" << js_code_.length();
}



std::string Initializer::GetJsCode() {
  return js_code_;
}


Initializer* Initializer::GetInstance() {
  return base::Singleton<Initializer>::get();
}
