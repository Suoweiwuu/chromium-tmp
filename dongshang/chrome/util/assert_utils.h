#ifndef DONG_SHANG_CHROME_UTIL_ASSERT_UTILS_H_
#define DONG_SHANG_CHROME_UTIL_ASSERT_UTILS_H_

#include <string>

class AssertUtils {
 public:
  static void IsTrue(bool result, std::string message);

  static void NotNull(void* obj, std::string message);

};

#endif  // DONG_SHANG_CHROME_BROWSER_WEBSOCKET_SERVER_H_
