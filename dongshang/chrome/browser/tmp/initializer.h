#ifndef DONG_SHANG_CHROME_BROWSER_INITIALIZER_H_
#define DONG_SHANG_CHROME_BROWSER_INITIALIZER_H_

#include <string>

class Initializer {
 public:
  Initializer();
  ~Initializer();

  bool initialize(std::string code);
};
#endif  // DONG_SHANG_CHROME_BROWSER_INITIALIZER_H_
