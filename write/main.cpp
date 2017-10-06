// write.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include <string>
#include "Common.h"

int main(int argc, char* argv[]) {
	std::string appPath = argv[0];				// ����·��
	std::string fileName = "";					// �ļ���
	std::vector<std::string> contentVec;		// ����
	bool isContent = false;
	int mode = 0;								// ������ӷ�ʽ(-1:�����ǰ��,0:�滻,1:����ں���)
	for (int i=1; i<argc; ++i) {
		std::string cmd = argv[i];
		if ("-pre" == cmd) {
			isContent = false;
			mode = -1;
		}
		if ("-rep" == cmd) {
			isContent = false;
			mode = 0;
			continue;
		}
		if ("-suf" == cmd) {
			isContent = false;
			mode = 1;
			continue;
		}
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
	std::string content = "";
	for (size_t i=0; i<contentVec.size(); ++i) {
		content += contentVec[i];
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
	if (-1 == mode) {			// �����ǰ��
		str = content + str;
	} else if (1 == mode) {		// ����ں���
		str = str + content;
	} else {					// �滻
		str = content;
	}
	Common::writeDataToFile((const unsigned char*)str.c_str(), str.size(), fileName);
	return 0;
}

