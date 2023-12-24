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


typedef struct _hArchive
{
	std::string _basePath{};
	std::string _relPath{};
	uint64_t _index{ 0 };
} hArch;

typedef hArch* PHARCH;

namespace ArcSnp {
	fProp GetMetadata(const std::string& filePath, const std::string& logFilePath);
	DWORD OpenArchive(PHARCH& archiveHandle, const std::string& path);
	bit7z::buffer_t GetBuffer(PHARCH& archiveHandle);
	std::map<const std::string, bit7z::buffer_t> GetContent(bit7z::buffer_t archBuffer);
}