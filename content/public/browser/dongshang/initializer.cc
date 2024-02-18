#define MY_BASE_IMPLEMENTATION

#include <string>
#include <vector>
#include "content/public/browser/dongshang/initializer.h"
#include "base/files/file_util.h"
#include "base/files/file_enumerator.h"
#include "base/memory/singleton.h"


void Initializer::InitJsCode() {
  std::string directory(u8"./nice-assistant");

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

Initializer* Initializer::LocalGetInstance() {
  return base::Singleton<Initializer>::get();
}
