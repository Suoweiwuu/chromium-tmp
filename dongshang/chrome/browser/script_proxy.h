#ifndef DONG_SHANG_CHROME_BROWSER_SCRIPT_PROXY_H_
#define DONG_SHANG_CHROME_BROWSER_SCRIPT_PROXY_H_

#include <string>
#include "base/values.h"

class ScriptProxy {
 public:
  ScriptProxy();
  ~ScriptProxy();

  bool ExecuteJsCode(std::string code);

  void ExecuteJsCodeCallback(base::Value result);

  static ScriptProxy* GetInstance();

 private:
  base::WeakPtrFactory<ScriptProxy> weak_factory_{this};

};
#endif  // DONG_SHANG_CHROME_BROWSER_SCRIPT_PROXY_H_
