// merge.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include <string>
#include "Common.h"

int main(int argc, char* argv[]) {
	std::string appPath = argv[0];				// ����·��
	std::string fileName = "";					// Ŀ���ļ���
	bool newLineFlag = false;					// �ļ������Ƿ���
	std::vector<std::string> fileVec;			// �ļ���
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

