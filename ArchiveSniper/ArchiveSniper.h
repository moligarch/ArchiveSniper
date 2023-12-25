#pragma once
#include <iostream>
#include <string>
#include <vector>

#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitarchiveeditor.hpp>


struct DCOMP
{
	std::string msBasePath{};
	DWORD msDepth{};
	uint64_t msSize{};
	bit7z::buffer_t msBuffer{};
};
using content_t = std::vector<DCOMP>;

struct META
{
	uint32_t itemsCount{};
	uint32_t foldersCount{};
	uint32_t filesCount{};
	uint64_t size{};
	uint64_t packSize{};
	bit7z::byte_t format{};
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

	META GetMetadata(const std::string& filePath, const std::string& logFilePath);
	content_t GetContent(std::string& path);
	//std::vector<std::string> GetList(std::string& path);
	//DWORD RemoveContent(bit7z::buffer_t archBuffer);

private:
	content_t ResolveBuffer(const std::string& basePath, bit7z::buffer_t& buffer);
	bool ValidateFile(const bit7z::BitArchiveItemInfo& itemInfo);
	std::shared_ptr<bit7z::Bit7zLibrary> mLib;
	bool mRecursion = false;
	DWORD mDepthLimit;
	DWORD mDepth{1};
	std::string mPath;
	const DWORD kBaseArchiveLevel{ 1 };
	const DWORD kMaxFileBufferSize{ 10000 };		//10000 KB ~ 10MB
};