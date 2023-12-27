#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <bit7z/bitarchivereader.hpp>

struct DCOMP
{
	std::string msBasePath{};
	DWORD msDepth{};
	bit7z::buffer_t msBuffer{};
};

using content_t = std::vector<DCOMP>;

struct META
{
	uint32_t msItemsCount{};
	uint32_t msFoldersCount{};
	uint32_t msFilesCount{};
	uint64_t msSize{};
	uint64_t msPackSize{};
	bit7z::byte_t msformat{};
	std::error_code msErrCode{};
	std::string msErrMsg{};
};

class ArcSnp
{
public:
	ArcSnp(const std::string& path, bool allowRecursive, DWORD depthLimit);
	ArcSnp(const std::string& path, bool allowRecursive);
	~ArcSnp();

	META GetMetadata(const std::string& filePath);

	/**********************************************************************
	* @brief Recursively retrieves the content from the specified path.
	*
	* @param path The path to the content.
	* @return The content extracted from the specified path as a vector of DCOMP (content_t).
	*			for more info look at DCOMP structure definition
	*
	* This method retrieves the buffer of every insider file of one archive from the specified file path,
	*			supporting recursive extraction for supported compressed formats.
	**********************************************************************/
	content_t GetContent(std::string& path);

	/*TODO
	//std::vector<std::string> GetList(std::string& path);
	//DWORD RemoveContent(bit7z::buffer_t archBuffer);
	*/
private:
	class ArcSnpImpl;
	std::unique_ptr<ArcSnpImpl> mpImpl;
};