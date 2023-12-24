#pragma once
#include <iostream>
#include <string>
#include <vector>

#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitarchiveeditor.hpp>

typedef struct _fProp
{
	uint32_t itemsCount{};
	uint32_t foldersCount{};
	uint32_t filesCount{};
	uint64_t size{};
	uint64_t packSize{};
	bit7z::byte_t format{};
} fProp;


typedef struct _hArchive
{
	std::string _basePath{};
	std::string _relPath{};
	uint64_t _index{ 0 };
} hArch;

typedef hArch* PHARCH;
typedef std::vector<bit7z::buffer_t> arch_t;
typedef std::map<const std::string, bit7z::buffer_t> content_t;

namespace ArcSnp {
	fProp GetMetadata(const std::string& filePath, const std::string& logFilePath);
	DWORD OpenArchive(PHARCH& hArch, const std::string& path);
	bit7z::buffer_t GetBuffer(PHARCH& hArch);
	std::map<const std::string, bit7z::buffer_t> GetContent(bit7z::buffer_t archBuffer);
	DWORD ClearBuffer(bit7z::buffer_t archBuffer);
	//Recursive
	arch_t GetBufferR(PHARCH& hArch, const DWORD dwLevel);
	content_t GetContentR(bit7z::buffer_t archBuffer, const DWORD dwLevel);
}