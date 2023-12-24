#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <bit7z/bitarchivereader.hpp>

typedef struct _fProp
{
	uint32_t itemsCount{};
	uint32_t foldersCount{};
	uint32_t filesCount{};
	uint64_t size{};
	uint64_t packSize{};
	unsigned char format{};
} fProp;


namespace ArcSnp {
	fProp GetMetadata(const std::string& filePath, const std::string& logFilePath);

}