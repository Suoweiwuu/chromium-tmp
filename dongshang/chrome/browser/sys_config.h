#ifndef DONG_SHANG_CHROME_BROWSER_SYS_CONFIG_H_
#define DONG_SHANG_CHROME_BROWSER_SYS_CONFIG_H_

#include <string>
#include "base/values.h"

class SysConfig {
 public:
  SysConfig();
  ~SysConfig();

  static ScriptProxy* GetInstance();

 private:
  
  std::string source_code_path_;

  std::string git_repo_url_;

  base::WeakPtrFactory<SysConfig> weak_factory_{this};

};
#endif  // DONG_SHANG_CHROME_BROWSER_SYS_CONFIG_H_
