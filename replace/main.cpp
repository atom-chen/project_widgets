// replace.cpp : 定义控制台应用程序的入口点。
//
#include <string>
#include "Common.h"

int main(int argc, char* argv[]) {
	std::string appPath = argv[0];				// 程序路径
	std::string fileName = "";					// 文件名
	std::vector<std::string> contentVec;		// 内容
	bool isContent = false;
	for (int i=1; i<argc; ++i) {
		std::string cmd = argv[i];
		if ("-file" == cmd && i + 1<argc) {
			isContent = false;
			fileName = argv[i + 1];
			continue;
		}
		if ("-str" == cmd) {
			isContent = true;
			continue;
		}
		if (isContent) {
			contentVec.push_back(argv[i]);
		}
	}
	if (fileName.empty()) {
		printf("no specify write file !!!\n");
		return 0;
	}
	// 
	fileName = Common::getFullPath(fileName);
	if (!Common::existFile(fileName) && !Common::createFile(fileName)) {
		printf("not exist file: [%s] and create failed !!!\n", fileName.c_str());
		return 0;
	}
	long fileSize;
	char* data = (char*)Common::getFileData(fileName, &fileSize);
	std::string str("");
	if (data) {
		str = data;
		delete data;
		data = NULL;
	}
	bool isMatch = 0 == contentVec.size()%2;
	size_t count = isMatch ? contentVec.size() : contentVec.size() - 1;
	for (size_t i=0; i<count;) {
		std::string src = contentVec[i];
		std::string dest = contentVec[i + 1];
		str = Common::replaceString(str, src, dest);
		i += 2;
	}
	if (count < contentVec.size()) {
		std::string src = contentVec[count];
		std::string dest = "";
		str = Common::replaceString(str, src, dest);
	}
	Common::writeDataToFile((const unsigned char*)str.c_str(), str.size(), fileName);
	return 0;
}

