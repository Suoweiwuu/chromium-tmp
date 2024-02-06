#include "file_utils.h"
#include <fstream>  
#include <sstream> 
#include <io.h>
#include <vector>
#include <numeric>
#include <iostream>
#include <filesystem>


using namespace std;

std::string FileUtils::ReadStrFromFile(std::string path) {
  std::vector<string> lines;
  string line;

  ifstream input_file(path);

  while (getline(input_file, line)) {
    lines.push_back(line + "\n");
  }

  std::string strData;
  strData = std::accumulate(lines.begin(), lines.end(), strData);
  return strData;
}

void FileUtils::GetAllFiles(string path, vector<string>& files) {
  intptr_t hFile = 0;  // �ļ����  64λ��������intptr_t��32λ��������long
  struct _finddata_t fileinfo;  // �ļ���Ϣ
  string p;
  if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)  // �ļ�����
  {
    do {
      if ((fileinfo.attrib & _A_SUBDIR))  // �ж��Ƿ�Ϊ�ļ���
      {
        if (strcmp(fileinfo.name, ".") != 0 &&
            strcmp(fileinfo.name, "..") != 0)  // �ļ������в���"."��".."
        {
          //  files.push_back(p.assign(path).append("\\").append(fileinfo.name));
          //  //�����ļ�����
          GetAllFiles(p.assign(path).append("\\").append(fileinfo.name), files);  // �ݹ�����ļ���
        }
      } else {
          files.push_back(p.assign(path).append("\\").append(fileinfo.name));  // ��������ļ��У������ļ���
      }
    } while (_findnext(hFile, &fileinfo) == 0);
    _findclose(hFile);
  }
}
