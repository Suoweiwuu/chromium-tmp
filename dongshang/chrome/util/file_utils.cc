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
  intptr_t hFile = 0;  // 文件句柄  64位下类型是intptr_t，32位下类型是long
  struct _finddata_t fileinfo;  // 文件信息
  string p;
  if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)  // 文件存在
  {
    do {
      if ((fileinfo.attrib & _A_SUBDIR))  // 判断是否为文件夹
      {
        if (strcmp(fileinfo.name, ".") != 0 &&
            strcmp(fileinfo.name, "..") != 0)  // 文件夹名中不含"."和".."
        {
          //  files.push_back(p.assign(path).append("\\").append(fileinfo.name));
          //  //保存文件夹名
          GetAllFiles(p.assign(path).append("\\").append(fileinfo.name), files);  // 递归遍历文件夹
        }
      } else {
          files.push_back(p.assign(path).append("\\").append(fileinfo.name));  // 如果不是文件夹，储存文件名
      }
    } while (_findnext(hFile, &fileinfo) == 0);
    _findclose(hFile);
  }
}
