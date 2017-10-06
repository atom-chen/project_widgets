// data2lua.cpp : 定义控制台应用程序的入口点。
//

#include <iostream>
#include <string>
#include "Common.h"
#include "BasicExcel.hpp"
#include "rapidxml.hpp"
#include "fcsv.h"

bool excel2lua(std::string excelName, std::string sheetName, std::string luaName) {
	YExcel::BasicExcel excel;
	if (!excel.Load(excelName.c_str())) {
		printf("can't open file \"%s\" !!!\n", excelName.c_str());
		return false;
	}
	YExcel::BasicExcelWorksheet* sheet = excel.GetWorksheet(sheetName.c_str());
	if (NULL == sheet) {
		sheet = excel.GetWorksheet("Sheet1");
		if (NULL == sheet) {
			printf("can't find sheet \"%s\" and \"Sheet1\" in file \"%s\" !!!\n", sheetName.c_str(), excelName.c_str());
			return false;
		}
	}
	size_t rowCount = sheet->GetTotalRows();
	size_t colCount = sheet->GetTotalCols();
	if (0 == rowCount || 0 == colCount) {
		printf("sheet \"%s\" in file \"%s\" is empty, row = %d, col = %d !!!\n", sheetName.c_str(), excelName.c_str(), rowCount, colCount);
		return false;
	}
	std::string content = "return{\n";
	for (size_t row=0; row<rowCount; ++row) {
		content = content + "{";
		for (size_t col=0; col<colCount; ++col) {
			content = content + (0 == col ? "" : ",") + "\"";
			YExcel::BasicExcelCell* cell = sheet->Cell(row, col);
			switch (cell->Type()) {
			case YExcel::BasicExcelCell::UNDEFINED: {
					printf("cell at (%d, %d) is empty !!!", row + 1, col + 1);
					break;
				}
			case YExcel::BasicExcelCell::INT: {
					int value = cell->GetInteger();
					content = content + Common::toString(value);
					break;
				}
			case YExcel::BasicExcelCell::DOUBLE: {
					double value = cell->GetDouble();
					content = content + Common::toString(value);
					break;
				}
			case YExcel::BasicExcelCell::STRING: {
					std::string value = cell->GetString();
					content = content + value;
					break;
				}
			case YExcel::BasicExcelCell::WSTRING: {
					std::wstring value = cell->GetWString();
					content = content + Common::wstring2string(value);
					break;
				}
			}
			content = content + "\"";
		}
		content = content + "}" + (row + 1 == rowCount ? "" : ",") + "\n";
	}
	content = content + "}";
	Common::writeDataToFile((const unsigned char*)content.c_str(), content.size(), luaName);
	return true;
}

bool xml2lua(std::string xmlName, std::string luaName, bool isOrder) {
	if (!Common::existFile(xmlName)) {
		printf("can't open file \"%s\" !!!\n", xmlName.c_str());
		return false;
	}
	long fileSize;
	unsigned char* data = Common::getFileData(xmlName, &fileSize);
	if (NULL == data || 0 == fileSize) {
		printf("file \"%s\" is empty !!!\n", xmlName.c_str());
		return false;
	}
	rapidxml::xml_document<> doc;
	doc.parse<0>(const_cast<char*>((char*)data));
	rapidxml::xml_node<>* root = doc.first_node();
	unsigned int row = 0;
	size_t rowCount = root->value_size();
	std::string content = "return{\n";
	for (rapidxml::xml_node<>* rowNode = root->first_node(); rowNode; rowNode = rowNode->next_sibling()) {
		content = content + (0 == row ? "" : ",\n") + "{";
		size_t col = 0;
		if (isOrder) {
			if (0 == row) {
				for (rapidxml::xml_node<>* colNode = rowNode->first_node(); colNode; colNode = colNode->next_sibling()) {
					content = content + (0 == col ? "" : ",");
					content = content + "\"" + colNode->name() + "\"";
					++col;
				}
				content = content + "},\n{";
				col = 0;
			}
			for (rapidxml::xml_node<>* colNode = rowNode->first_node(); colNode; colNode = colNode->next_sibling()) {
				content = content + (0 == col ? "" : ",");
				content = content + "\"" + colNode->value() + "\"";
				++col;
			}
		} else {
			for (rapidxml::xml_node<>* colNode = rowNode->first_node(); colNode; colNode = colNode->next_sibling()) {
				content = content + (0 == col ? "" : ",");
				content = content + "[\"" + colNode->name() + "\"]=\"" + colNode->value() + "\"";
				++col;
			}
		}
		content = content + "}";
		++row;
	}
	doc.clear();
	content = content + "\n}";
	Common::writeDataToFile((const unsigned char*)content.c_str(), content.size(), luaName);
	return true;
}

bool csv2lua(std::string csvName, std::string luaName) {
	if (!Common::existFile(csvName)) {
		printf("can't open file \"%s\" !!!\n", csvName.c_str());
		return false;
	}
	long fileSize;
	unsigned char* data = Common::getFileData(csvName, &fileSize);
	if (NULL == data || 0 == fileSize) {
		printf("file \"%s\" is empty !!!\n", csvName.c_str());
		return false;
	}
	struct fCSV* csv = fcsv_read((char*)data, fileSize);
	if (NULL == csv) {
		fcsv_close(csv);
		delete data;
		data = NULL;
		printf("read file \"%s\" failed !!!\n", csvName.c_str());
		return false;
	}
	std::vector<const char*> names;
	struct fRow* rowNode = fcsv_first_row(csv);
	unsigned int rowCount = fcsv_rows_count(csv);
	std::string content = "return{\n";
	for (unsigned int row=0; row<rowCount; ++row) {
		unsigned int colCount = fcsv_fields_count(rowNode);
		content = content + "{";
		for (unsigned int col=0; col<colCount; ++col) {
			content = content + (0 == col ? "" : ",") + "\"";
			const char* field = fcsv_get_field(rowNode, col);
			content = content + field;
			content = content + "\"";
		}
		content = content + "}" + (row + 1 == rowCount ? "" : ",") + "\n";
		rowNode = fcsv_next_row(rowNode);
	}
	content = content + "}";
	Common::writeDataToFile((const unsigned char*)content.c_str(), content.size(), luaName);
	fcsv_close(csv);
	delete data;
	data = NULL;
	return true;
}

int main(int argc, char* argv[]) {
	std::string appPath = argv[0];				// 程序路径
	std::string searchPath = "";				// 搜索路径
	std::string file = "";						// 文件名
	std::vector<std::string> extVec;			// 文件类型
	bool extFlag = false;						// 文件扩展名标识
	// 参数解析
	for (int i=1; i<argc; ++i) {
		std::string cmd = argv[i];
		if ("-dir" == cmd && i + 1<argc) {
			extFlag = false;
			searchPath = argv[i + 1];
			continue;
		}
		if ("-file" == cmd && i + 1<argc) {
			extFlag = false;
			file = argv[i + 1];
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
	std::vector<std::string> fileVec;
	std::string fullPath = Common::getFullPath(searchPath);
	if (!file.empty()) {
		fileVec.push_back(fullPath + "/" + file);
	} else if (!extVec.empty()) {
		Common::searchFile(fullPath, extVec, fileVec);
	}
	for (size_t i=0; i<fileVec.size(); ++i) {
		std::vector<std::string> fileInfo = Common::stripFileInfo(fileVec[i]);
		if (4 != fileInfo.size()) {
			continue;
		}
		if (".xls" == fileInfo[3])
		{
			excel2lua(fileVec[i], fileInfo[2], fileInfo[0] + fileInfo[2] + ".lua");
		}
		else if (".xml" == fileInfo[3])
		{
			xml2lua(fileVec[i], fileInfo[0] + fileInfo[2] + ".lua", true);
		}
		else if (".csv" == fileInfo[3])
		{
			csv2lua(fileVec[i], fileInfo[0] + fileInfo[2] + ".lua");
		}
	}
	return 0;
}

