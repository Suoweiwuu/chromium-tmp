#ifndef CONTENT_PUBLIC_BROWSER_DONG_SHANG_INITIALIZER_H_
#define CONTENT_PUBLIC_BROWSER_DONG_SHANG_INITIALIZER_H_

#include <string>


#if defined(_MSC_VER)

#if defined(MY_BASE_IMPLEMENTATION)
#define MY_EXPORT __declspec(dllexport)
#else
#define MY_EXPORT __declspec(dllimport)
#endif

#else
#define MY_EXPORT __attribute__((visibility("default")))

#endif  // defined(BASE_IMPLEMENTATION)

class Initializer {
 public:
  Initializer() = default;
  ~Initializer() = default;

  MY_EXPORT void InitJsCode();

  std::string GetJsCode();

  MY_EXPORT static Initializer* GetInstance();

 private:
  std::string js_code_;
};
#endif  // CONTENT_PUBLIC_BROWSER_DONG_SHANG_INITIALIZER_H_
