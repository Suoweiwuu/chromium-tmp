#ifndef DONG_SHANG_CHROME_BROWSER_CHROME_BROWSER_MAIN_EXTRA_PARTS_DS_H_
#define DONG_SHANG_CHROME_BROWSER_CHROME_BROWSER_MAIN_EXTRA_PARTS_DS_H_

#include "chrome/browser/chrome_browser_main_extra_parts.h"

class ChromeBrowserMainExtraPartsDs : public ChromeBrowserMainExtraParts {
 public:
  ChromeBrowserMainExtraPartsDs();
  ~ChromeBrowserMainExtraPartsDs() override;

  ChromeBrowserMainExtraPartsDs(const ChromeBrowserMainExtraPartsDs&) = delete;
  ChromeBrowserMainExtraPartsDs& operator=(
      const ChromeBrowserMainExtraPartsDs&) = delete;

 private:
  void PreMainMessageLoopRun() override;
};

#endif  // DONG_SHANG_CHROME_BROWSER_CHROME_BROWSER_MAIN_EXTRA_PARTS_DS_H_
