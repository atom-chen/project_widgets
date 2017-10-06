// crypto.cpp : 定义控制台应用程序的入口点。
//
#include <iostream>
#include <string>
#include "Common.h"
#include "AES.h"
#include "RC4.h"
#include "xxtea.h"

// 算法类型
enum ALGORITHM_TYPE {
	AT_BEGIN = 0,			// begin of type
	AT_XOR,					// XOR
	AT_AES,					// AES
	AT_RC4,					// RC4
	AT_XXTEA,				// xxtea
	AT_END					// end of type
};

// 操作类型
enum  CRYPTO_TYPE {
	CT_BEGIN = 0,			// begin of type
	CT_ENCRYPT,				// encrypt
	CT_DECRYPT,				// decrypt
	CT_END					// end of type
};

// 加密文件
void encryptFile(const std::string& srcFile, const std::string& destFile, ALGORITHM_TYPE algorithm, unsigned char* key, unsigned int keyLength, unsigned char* sign, unsigned int signLength) {
	if (srcFile.empty() || destFile.empty()) {
		return;
	}
	long fileSize;
	unsigned char* data = Common::getFileData(srcFile, &fileSize);
	if (NULL == data) {
		return;
	}
	unsigned char* resultData = NULL;
	switch (algorithm) {
	case AT_XOR:
		resultData = Common::simpleXOR(data, key);
		break;
	case AT_AES:
		resultData = (unsigned char*)aes_encrypt(data, fileSize, key);
		break;
	case AT_RC4:
		resultData = rc4_crypto(data, fileSize, key);
		break;
	case AT_XXTEA:
		unsigned int resultSize;
		resultData = xxtea_encrypt(data, fileSize, key, keyLength, &resultSize);
		fileSize = resultSize;
		delete data;
		break;
	default:
		printf("algorithm code: %d is wrong when encrypt file %s", algorithm, srcFile);
		break;
	}
	if (NULL == sign || 0 == strcmp("", (char*)sign) || 0 == signLength) {
		Common::writeDataToFile(resultData, fileSize, destFile);
	} else {
		unsigned long tempLength = signLength + fileSize;
		unsigned char* tempData = new unsigned char[tempLength + 1];
		memcpy(tempData, sign, signLength);
		for (size_t i=0; i<fileSize; ++i) {
			tempData[signLength + i] = resultData[i];
		}
		tempData[tempLength] = '\0';
		Common::writeDataToFile(tempData, tempLength, destFile);
		delete tempData;
		tempData = NULL;
	}
	delete resultData;
	resultData = NULL;
	data = NULL;
}

// 解密文件
void decryptFile(const std::string& srcFile, const std::string& destFile, ALGORITHM_TYPE algorithm, unsigned char* key, unsigned int keyLength, unsigned char* sign, unsigned int signLength) {
	if (srcFile.empty() || destFile.empty()) {
		return;
	}
	long fileSize;
	unsigned char* data = Common::getFileData(srcFile, &fileSize);
	if (NULL == data) {
		return;
	}
	if (sign && 0 != strcmp("", (char*)sign) && signLength > 0) {
		fileSize = fileSize - signLength;
		unsigned char* tempData = new unsigned char[fileSize + 1];
		memcpy(tempData, data + signLength, fileSize);
		tempData[fileSize] = '\0';
		delete data;
		data = tempData;
	}
	unsigned char* resultData = NULL;
	switch (algorithm) {
	case AT_XOR:
		resultData = Common::simpleXOR(data, key);
		break;
	case AT_AES:
		resultData = (unsigned char*)aes_decrypt(data, fileSize, key);
		break;
	case AT_RC4:
		resultData = rc4_crypto(data, fileSize, key);
		break;
	case AT_XXTEA:
		unsigned int resultSize;
		resultData = xxtea_decrypt(data, fileSize, key, keyLength, &resultSize);
		fileSize = resultSize;
		delete data;
		break;
	default:
		printf("algorithm code: %d is wrong when decrypt file %s", algorithm, srcFile);
		break;
	}
	Common::writeDataToFile(resultData, fileSize, destFile);
	delete resultData;
	resultData = NULL;
	data = NULL;
}

int main(int argc, char* argv[]) {
	std::string appPath = argv[0];				// 程序路径
	std::string algorithmStr = "";				// 算法类型
	std::string cryptoStr = "";					// 加密/解密类型
	std::string keyStr = "";					// 秘钥
	std::string signStr = "";					// 签名
	std::string searchPath = "";				// 搜索路径
	std::vector<std::string> extVec;			// 文件类型
	bool extFlag = false;						// 文件扩展名标识
	// 参数解析
	for (int i=1; i<argc; ++i) {
		std::string cmd = argv[i];
		if ("-alg" == cmd && i + 1<argc) {
			extFlag = false;
			algorithmStr = argv[i + 1];
			continue;
		}
		if ("-op" == cmd && i + 1<argc) {
			extFlag = false;
			cryptoStr = argv[i + 1];
			continue;
		}
		if ("-key" == cmd && i + 1<argc) {
			extFlag = false;
			keyStr = argv[i + 1];
			continue;
		}
		if ("-sign" == cmd && i + 1<argc) {
			extFlag = false;
			signStr = argv[i + 1];
			continue;
		}
		if ("-dir" == cmd && i + 1<argc) {
			extFlag = false;
			searchPath = argv[i + 1];
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
	printf("crypto path: [%s] \n", searchPath.c_str());
	// 参数检查
	int algorithmNum = Common::toInt(algorithmStr);
	if (algorithmNum <= AT_BEGIN || algorithmNum >= AT_END) {
		printf("algorithm string: [%s] is wrong !!!\n", algorithmStr.c_str());
		return 0;
	}
	int cryptoNum = Common::toInt(cryptoStr);
	if (cryptoNum <= CT_BEGIN || cryptoNum >= CT_END) {
		printf("crypto string: [%s] is wrong !!!\n", cryptoStr.c_str());
		return 0;
	}
	if (keyStr.empty()) {
		printf("key is empty !!!\n");
		return 0;
	}
	// 秘钥拷贝
	unsigned long keyLength = keyStr.size();
	unsigned char* key = new unsigned char[keyLength + 1];
	memcpy(key, keyStr.c_str(), keyLength);
	key[keyLength] = '\0';
	// 签名拷贝
	unsigned long signLength = signStr.size();
	unsigned char* sign = NULL;
	if (signLength > 0)
	{
		sign = new unsigned char[signLength + 1];
		memcpy(sign, signStr.c_str(), signLength);
		sign[signLength] = '\0';
	}
	// 执行操作
	std::vector<std::string> fileVec;
	Common::searchFile(Common::getFullPath(searchPath), extVec, fileVec);
	for (size_t i=0; i<fileVec.size(); ++i) {
		std::string filePath = fileVec[i];
		switch ((CRYPTO_TYPE)cryptoNum) {
		case CT_ENCRYPT:
			encryptFile(filePath, filePath, (ALGORITHM_TYPE)algorithmNum, key, keyLength, sign, signLength);
			break;
		case CT_DECRYPT:
			decryptFile(filePath, filePath, (ALGORITHM_TYPE)algorithmNum, key, keyLength, sign, signLength);
			break;
		default:
			break;
		}
	}
	delete key;
	key = NULL;
	delete sign;
	sign = NULL;
	printf("crypto success ...\n");
	return 0;
}

