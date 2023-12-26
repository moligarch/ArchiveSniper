#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>

#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitarchiveeditor.hpp>

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
	class sLib {
	public:
		static sLib& Instance() {
			static sLib instance;
			return instance;
		}
		std::shared_ptr<bit7z::Bit7zLibrary> LibAccess() {
			return msLib;
		}
		sLib(sLib const&) = delete;
		sLib(sLib &&) = delete;
		void operator=(sLib const&) = delete;
		void operator=(sLib &&) = delete;
	private:
		sLib() {
			if (!msLib)
			{
				msLib = std::make_shared<bit7z::Bit7zLibrary>();
			}
		};

		std::shared_ptr<bit7z::Bit7zLibrary> msLib;
	};
public:
	explicit ArcSnp(const std::string& path, bool allowRecursive, DWORD depthLimit = 2);
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

	/**********************************************************************
	* @brief Resolves the buffer content and store it into a vector of DCOMP structures (content_t).
	*
	* @param basePath : The base path where the buffer is located.
	*			'->' notation means the buffer/file before it, got opened and anything after '->' is inside that
	* @param buffer : The buffer to be resolved.
	* @return The content extracted from the buffer as a content_t.
	*
	* This method takes a buffer and its base path, and recursively extracts its content into a content_t.
	* Each DCOMP structure contains the base path, the depth to show recursion level, and the corresponding buffer for that file.
	**********************************************************************/
	content_t ResolveBuffer(const std::string& basePath, bit7z::buffer_t& buffer);

	/**********************************************************************
	* @brief Checks if the file format of the given item is supported and if its size is under 10 MB.
	*
	* This method takes an item (referring to a file) and checks if its format is supported. It then
	* verifies if the file size is under 10 MB.
	*
	* @param itemInfo The information of the file to be checked, including its format and size.
	* @return DWORD Returns 0 if the file format is supported and its size is under 10 MB.
	* Returns 1 if the file format is unsupported. Returns 2 if the file size is greater than or equal to 10 MB.
	**********************************************************************/
	bool CheckBuffer(const bit7z::BitArchiveItemInfo& itemInfo);

	std::shared_ptr<bit7z::Bit7zLibrary> mLib;
	bool mRecursion = false;
	DWORD mDepthLimit;
	DWORD mDepth{0};
	std::string mPath;
	const DWORD kBaseArchiveLevel{ 0 };
	const DWORD kMaxFileBufferSize{ 10000 };		//10000 KB ~ 10MB
};