// traverse.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include <string>
#include "Common.h"

std::string getName(std::string filePath, std::string cutTag) {
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
	std::string appPath = argv[0];				// ����·��
	int searchType = 1;							// ��������:1.Ŀ¼,2.�ļ�
	int recursionType = 1;						// �ݹ�:0.���ݹ�,1.�ݹ�
	std::string searchPath = "";				// ����·��
	std::string cutTag = "";					// ��ȡ��ʶ
	std::vector<std::string> extVec;			// �ļ�����,��2 == searchTypeʱ��Ч
	bool extFlag = false;
	for (int i=1; i<argc; ++i) {
		std::string cmd = argv[i];
		if ("-type" == cmd && i + 1<argc) {
			extFlag = false;
			searchType = Common::toInt(argv[i + 1]);
			continue;
		}
		if ("-recursion" == cmd && i + 1<argc) {
			extFlag = false;
			recursionType = Common::toInt(argv[i + 1]);
			continue;
		}
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
	if (1 != searchType && 2 != searchType) {
		printf("type %d is error, must in [1,2] !!!\n", searchType);
		return 0;
	}
	if (0 != recursionType && 1 != recursionType) {
		printf("recursion %d is error, must in [0,1] !!!\n", recursionType);
		return 0;
	}
	searchPath = Common::getFullPath(searchPath);
	std::string fileContent = "";
	if (1 == searchType) {			// ����Ŀ¼
		std::vector<std::string> dirVec;
		Common::searchDir(searchPath, dirVec, 1 == recursionType);
		for (size_t i=0; i<dirVec.size(); ++i) {
			if (!fileContent.empty()) {
				fileContent += Common::newLineString();
			}
			fileContent += getName(dirVec[i], cutTag);
		}
	} else if (2 == searchType) {	// �����ļ�
		std::vector<std::string> fileVec;
		Common::searchFile(searchPath, extVec, fileVec, 1 == recursionType);
		for (size_t i=0; i<fileVec.size(); ++i) {
			if (!fileContent.empty()) {
				fileContent += Common::newLineString();
			}
			fileContent += getName(fileVec[i], cutTag);
		}
	}
	Common::writeDataToFile((const unsigned char*)fileContent.c_str(), fileContent.size(), searchPath + "/" + "TraverseList.txt");
	return 0;
}

