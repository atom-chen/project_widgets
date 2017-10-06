// merge.cpp : 定义控制台应用程序的入口点。
//
#include <string>
#include "Common.h"

int main(int argc, char* argv[]) {
	std::string appPath = argv[0];				// 程序路径
	std::string fileName = "";					// 目标文件名
	bool newLineFlag = false;					// 文件内容是否换行
	std::vector<std::string> fileVec;			// 文件名
	bool isFile = false;
	for (int i=1; i<argc; ++i) {
		std::string cmd = argv[i];
		if ("-file" == cmd && i + 1<argc) {
			isFile = false;
			fileName = argv[i + 1];
		}
		if ("-newline" == cmd) {
			isFile = false;
			newLineFlag = true;
		}
		if ("-list" == cmd) {
			isFile = true;
			continue;
		}
		if (isFile) {
			fileVec.push_back(argv[i]);
		}
	}
	if (fileVec.empty()) {
		printf("no specify write file !!!\n");
		return 0;
	}
	//
	std::string content = "";
	for (size_t i=0; i<fileVec.size(); ++i) {
		std::string fullFileName = Common::getFullPath(fileVec[i]);
		long fileSize;
		char *data = (char*)Common::getFileData(fullFileName, &fileSize);
		if (data) {
			if (0 == content.size()) {
				content = data;
			} else {
				content += (newLineFlag ? Common::newLineString() : "") + data;
			}
			delete data;
			data = NULL;
		} else {
			printf("not exist file: [%s] and create failed !!!\n", fullFileName.c_str());
		}
	}
	Common::writeDataToFile((const unsigned char*)content.c_str(), content.size(), fileName);
	return 0;
}

