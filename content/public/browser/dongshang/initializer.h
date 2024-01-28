#ifndef CONTENT_PUBLIC_BROWSER_DONG_SHANG_INITIALIZER_H_
#define CONTENT_PUBLIC_BROWSER_DONG_SHANG_INITIALIZER_H_

#include <string>

class Initializer {
 public:
  Initializer();
  ~Initializer();

  bool initialize(std::string code);
};
#endif  // CONTENT_PUBLIC_BROWSER_DONG_SHANG_INITIALIZER_H_
