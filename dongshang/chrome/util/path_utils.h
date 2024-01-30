#ifndef DONG_SHANG_CHROME_UTIL_PATH_UTILS_H_
#define DONG_SHANG_CHROME_UTIL_PATH_UTILS_H_

#include <string>

class PathUtils {
 public:
  static std::string GetFileFullName(std::string path);
  static std::string GetFileNameWithoutExtension(std::string path);
};

#endif  // DONG_SHANG_CHROME_BROWSER_WEBSOCKET_SERVER_H_
