#ifndef DONG_SHANG_CHROME_UTIL_FILE_UTILS_H_
#define DONG_SHANG_CHROME_UTIL_FILE_UTILS_H_

#include <string>

class FileUtils {
 public:
  static std::string ReadStrFromFile(std::string path);

  static void GetAllFiles(std::string path, std::vector<std::string>& files);
};

#endif  // DONG_SHANG_CHROME_BROWSER_WEBSOCKET_SERVER_H_
