// autoincrement.cpp : 定义控制台应用程序的入口点。
//
#include <string>
#include <Common.h>

int main(int argc, char* argv[]) {
	std::string appPath = argv[0];				// 程序路径
	std::string fileName = "";					// 文件名
	for (int i=1; i<argc; ++i) {
		std::string cmd = argv[i];
		if ("-file" == cmd && i + 1<argc) {
			fileName = argv[i + 1];
			break;
		}
	}
	if (fileName.empty()) {
		printf("no specify autoincrement file !!!\n");
		return 0;
	}
	fileName = Common::getFullPath(fileName);
	long fileSize;
	char* data = (char*)Common::getFileData(fileName, &fileSize);
	std::string originalStr("0");
	if (data) {
		originalStr = data;
		delete data;
		data = NULL;
	}
	std::string str = Common::replaceString(originalStr, "\r", "");
	str = Common::replaceString(str, "\n", "");
	str = Common::replaceString(str, "\t", "");
	str = Common::replaceString(str, " ", "");
	if (!Common::isDigit(str)) {
		printf("file data [%s] is error, can't change to digit type !!!\n", originalStr.c_str());
		return 0;
	}
	int number = Common::toInt(str);
	++number;
	str = Common::toString(number);
	Common::writeDataToFile((const unsigned char*)str.c_str(), str.size(), fileName);
	return 0;
}
