// md5.cpp : 定义控制台应用程序的入口点。
//

#include <iostream>
#include <string>
#include "Common.h"
#include "MD5.h"

std::string generateMD5(const std::string& srcFile) {
	long fileLength = 0;
	char *data = (char*)Common::getFileData(srcFile, &fileLength);
	if (NULL == data) {
		return "";
	}
	std::string md5Value = MD5_sign((unsigned char*)data, fileLength);
	delete data;
	data = NULL;
	return md5Value;
}

std::string getMd5Name(std::string filePath, std::string cutTag) {
	if (cutTag.empty()) {
		return filePath;
	}
	std::string::size_type pos = filePath.find(cutTag);
	if (std::string::npos != pos) {
		std::string::size_type cutPos = pos + cutTag.size();
		filePath = filePath.substr(cutPos, filePath.npos);
	}
	return filePath;
}

int main(int argc, char* argv[]) {
	std::string appPath = argv[0];				// 程序路径
	std::string searchPath = "";				// 搜索路径
	std::string cutTag = "";					// 截取标识
	std::vector<std::string> extVec;			// 文件类型
	bool extFlag = false;
	for (int i=1; i<argc; ++i) {
		std::string cmd = argv[i];
		if ("-dir" == cmd && i + 1<argc) {
			extFlag = false;
			searchPath = argv[i + 1];
			continue;
		}
		if ("-cut" == cmd && i + 1<argc) {
			extFlag = false;
			cutTag = argv[i + 1];
			continue;
		}
		if ("-ext" == cmd) {
			extFlag = true;
			continue;
		}
		if (extFlag) {
			extVec.push_back(argv[i]);
		}
	}
	searchPath = Common::getFullPath(searchPath);
	std::vector<std::string> fileVec;
	Common::searchFile(searchPath, extVec, fileVec);
	// 生成md5值
	std::string fileContent = "";
	for (size_t i=0; i<fileVec.size(); ++i) {
		std::string filePath = fileVec[i];
		std::string md5Value = generateMD5(filePath);
		long fileSize = Common::calcFileSize(filePath);
		if ("" != fileContent) {
			fileContent += Common::newLineString();
		}
		fileContent += md5Value + "," + getMd5Name(filePath, cutTag) + "," + Common::toString(fileSize);
	}
	// 保存到文件
	Common::writeDataToFile((const unsigned char*)fileContent.c_str(), fileContent.size(), searchPath + "/" + "Md5ListFile.txt");
	return 0;
}

