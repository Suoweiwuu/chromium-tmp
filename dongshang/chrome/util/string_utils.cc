#include "dongshang/chrome/util/string_utils.h"


using namespace std;

vector<string> StringUtils::Split(const string& str, const string& separator) {
  vector<string> res;
  if (str == "")
    return res;
  //在字符串末尾也加入分隔符，方便截取最后一段
  string strs = str + separator;
  size_t pos = strs.find(separator);

  while (pos != strs.npos) {
    string temp = strs.substr(0, pos);
    res.push_back(temp);
    //去掉已分割的字符串,在剩下的字符串中进行分割
    strs = strs.substr(pos + 1, strs.size());
    pos = strs.find(separator);
  }

  return res;
}