#include "path_utils.h"

using namespace std;

std::string PathUtils::GetFileFullName(std::string path) {
  string::size_type pos = path.find_last_of('\\') + 1;
  string file_name = path.substr(pos, path.length() - pos);
  return file_name;
}

std::string PathUtils::GetFileNameWithoutExtension(std::string path) {
  string full_name = GetFileFullName(path);
  string file_name_without_ext = full_name.substr(0, full_name.rfind("."));
  return file_name_without_ext;
}


