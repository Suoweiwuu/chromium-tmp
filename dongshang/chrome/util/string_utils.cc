#include "dongshang/chrome/util/string_utils.h"


using namespace std;

vector<string> StringUtils::Split(const string& str, const string& separator) {
  vector<string> res;
  if (str == "")
    return res;
  //���ַ���ĩβҲ����ָ����������ȡ���һ��
  string strs = str + separator;
  size_t pos = strs.find(separator);

  while (pos != strs.npos) {
    string temp = strs.substr(0, pos);
    res.push_back(temp);
    //ȥ���ѷָ���ַ���,��ʣ�µ��ַ����н��зָ�
    strs = strs.substr(pos + 1, strs.size());
    pos = strs.find(separator);
  }

  return res;
}