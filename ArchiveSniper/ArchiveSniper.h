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


typedef struct _Archive
{
	std::string _basePath{};
	std::string _relPath{};
	int _index{};

	_Archive(const std::string& basePath, const std::string& relPath, const int index)
	{
		_basePath = basePath;
		_relPath = relPath;
		_index = index;
	};
} ARCH;

typedef std::vector<bit7z::buffer_t> arch_t;
typedef std::map<const std::string, bit7z::buffer_t> content_t;

namespace ArcSnp {
	fProp GetMetadata(const std::string& filePath, const std::string& logFilePath);
	void GetArchive(ARCH& archive, const std::string& path, DWORD index);
	bit7z::buffer_t GetBuffer(ARCH& archive);
	std::map<const std::string, bit7z::buffer_t> GetContent(bit7z::buffer_t archBuffer);
	DWORD ClearBuffer(bit7z::buffer_t archBuffer);
	//Recursive
	arch_t GetBufferR(ARCH& archive, const DWORD dwLevel);
	content_t GetContentR(bit7z::buffer_t archBuffer, const DWORD dwLevel);
}