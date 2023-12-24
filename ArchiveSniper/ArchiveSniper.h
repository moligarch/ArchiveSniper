#pragma once
#include <iostream>
#include <string>
#include <vector>

#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitarchiveeditor.hpp>

#define PARENT_LEVEL_ARCHIVE	0
#define FIRST_LEVEL_ARCHIVE		1
#define SECOND_LEVEL_ARCHIVE	2
#define DEPTH_LIMIT				3

typedef struct _Decompressed
{
	std::string basePath{};
	std::string relPath{};
	DWORD depth{};
	uint64_t size{};
	bit7z::buffer_t buffer{};
} DCOMP;

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
	std::string _fullPath{};
	DWORD _level{};
	bool _recursive{};
} ARCH;

typedef std::vector<bit7z::buffer_t> arch_t;
typedef std::map<const std::string, bit7z::buffer_t> content_t;

namespace ArcSnp {
	fProp GetMetadata(const std::string& filePath, const std::string& logFilePath);
	void GetArchive(ARCH& archive, const std::string& path);
	std::vector<DCOMP> GetBuffer(ARCH& archive, bit7z::buffer_t& buffer);
	content_t GetContent(bit7z::buffer_t archBuffer);
	DWORD ClearBuffer(bit7z::buffer_t archBuffer);
	//Recursive
	arch_t GetBufferR(ARCH& archive, const DWORD dwLevel);
	content_t GetContentR(bit7z::buffer_t archBuffer, const DWORD dwLevel);
}