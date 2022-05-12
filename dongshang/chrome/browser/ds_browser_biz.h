#ifndef DONG_SHANG_CHROME_BROWSER_DS_BROWSER_BIZ_H_
#define DONG_SHANG_CHROME_BROWSER_DS_BROWSER_BIZ_H_

#include "base/memory/weak_ptr.h"

class DsBrowserBiz {
 public:
  DsBrowserBiz();
  ~DsBrowserBiz();

  DsBrowserBiz(const DsBrowserBiz&) = delete;
  DsBrowserBiz& operator=(const DsBrowserBiz&) = delete;

  void PreMainMessageLoopRun();

 private:
  base::WeakPtrFactory<DsBrowserBiz> weak_factory_{this};
};

#endif  // DONG_SHANG_CHROME_BROWSER_DS_BROWSER_BIZ_H_
