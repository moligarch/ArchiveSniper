#include "ArchiveSniper.h"
#include <fstream>

ArcSnp::ArcSnp(const std::string& path, bool allowRecursive, DWORD depthLimit) :
    mPath(path), mRecursion(allowRecursive), mDepthLimit(depthLimit)
{
    mLib = sLib::Instance().LibAccess();
}

ArcSnp::~ArcSnp()
{
}

META ArcSnp::GetMetadata(const std::string& filePath, const std::string& logFilePath) {

    std::ofstream log(logFilePath);
    try { // bit7z classes can throw BitException objects

        bit7z::BitArchiveReader arc{ *mLib, filePath, bit7z::BitFormat::Auto};

        META result{ arc.itemsCount(),arc.foldersCount(), arc.filesCount(), arc.size(), arc.packSize(), arc.format().value() };
        // Printing archive metadata
        log << "Archive properties" << std::endl;
        log << "  Items count: " << result.itemsCount << std::endl;
        log << "  Folders count: " << result.foldersCount << std::endl;
        log << "  Files count: " << result.filesCount << std::endl;
        log << "  Size: " << result.size << std::endl;
        log << "  Packed size: " << result.packSize << std::endl;
        log << "  Packed size: " << result.format << std::endl << std::endl;

        // Printing the metadata of the archived items
        std::cout << "Archived items";
        auto arc_items = arc.items();
        for (auto& item : arc_items) {
            log << std::endl;
            log << "  Item index: " << item.index() << std::endl;
            log << "    Name: " << item.name() << std::endl;
            log << "    Extension: " << item.extension() << std::endl;
            log << "    Path: " << item.path() << std::endl;
            log << "    IsDir: " << item.isDir() << std::endl;
            log << "    Size: " << item.size() << std::endl;
            log << "    Packed size: " << item.packSize() << std::endl;
            log << "    CRC: " << item.crc() << std::endl;
        }

        return result;
    }
    catch (const bit7z::BitException& ex) { 
        log << ex.what();
        return META{};
    }
}

content_t ArcSnp::GetContent(std::string& path)
{
    if (mDepth++ >= mDepthLimit)
    {
        mRecursion = false;
    }
    content_t result;
    if (!mRecursion)
    {
        bit7z::BitArchiveReader arc{ *mLib , path, bit7z::BitFormat::Auto};
        auto arc_items = arc.items();
        for (const auto& item : arc_items) {
            bit7z::buffer_t buffer{};
            arc.extractTo(buffer, item.index());
            result.emplace_back(DCOMP{
                                path + "->" + item.path(),
                                kBaseArchiveLevel,
                                item.size(),
                                buffer });
        }
    }
    else
    {
        bit7z::BitArchiveReader arc{ *mLib, path, bit7z::BitFormat::Auto };
        auto arc_items = arc.items();
        for (const auto& item : arc_items) {
            if (ValidateFile(item))
            {   
                bit7z::buffer_t tmpBuffer{};
                arc.extractTo(tmpBuffer, item.index());
                auto output = ResolveBuffer(path + "->" + item.path(), tmpBuffer);
                result.insert(result.end(), output.begin(), output.end());
            }
        }
    }
    mDepth--;
    return result;
}

content_t ArcSnp::ResolveBuffer(const std::string& basePath, bit7z::buffer_t& buffer)
{
    if (mDepth++ >= mDepthLimit)
    {
        mRecursion = false;
    }

    content_t result;
    if (!mRecursion)
    {
        bit7z::BitArchiveReader arc{ *mLib, buffer, bit7z::BitFormat::Auto };
        auto arc_items = arc.items();
        for (const auto& item : arc_items) {
            bit7z::buffer_t tmpBuffer{};
            arc.extractTo(tmpBuffer, item.index());
            result.emplace_back(DCOMP{
                                basePath + "->" + item.path(),
                                mDepth,
                                item.size(),
                                tmpBuffer });
        }
    }
    else
    {
        bit7z::BitArchiveReader arc{ *mLib, buffer, bit7z::BitFormat::Auto };
        auto arc_items = arc.items();
        for (const auto& item : arc_items) {
            if (ValidateFile(item))
            {
                bit7z::buffer_t tmpBuffer{};
                arc.extractTo(tmpBuffer, item.index());
                auto output = ArcSnp::ResolveBuffer(basePath + "->" + item.path(), tmpBuffer);
                result.insert(result.end(), output.begin(), output.end());
            }
        }
    }
    mDepth--;
    return result;
}

bool ArcSnp::ValidateFile(const bit7z::BitArchiveItemInfo& itemInfo)
{
    bool state = itemInfo.size() / 1024 < kMaxFileBufferSize;
    return state;
}