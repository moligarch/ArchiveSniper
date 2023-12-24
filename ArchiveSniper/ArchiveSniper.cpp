#include "ArchiveSniper.h"
#include <fstream>

fProp ArcSnp::GetMetadata(const std::string& filePath, const std::string& logFilePath) {

    std::ofstream log(logFilePath);
    try { // bit7z classes can throw BitException objects

        bit7z::Bit7zLibrary lib{ "7z.dll" };
        bit7z::BitArchiveReader arc{ lib, filePath, bit7z::BitFormat::Auto};

        fProp result{ arc.itemsCount(),arc.foldersCount(), arc.filesCount(), arc.size(), arc.packSize(), arc.format().value() };
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
        return fProp{};
    }
}


void ArcSnp::GetBufferInfo(BINFO& bufferInfo, const std::string& path, bool allowRecursive)
{
    bufferInfo._recursive = allowRecursive;
    if (bufferInfo._basePath == "")
    {
        bufferInfo._basePath = path;
        bufferInfo._depth = PARENT_LEVEL_ARCHIVE;
    }
    else
    {
        if (bufferInfo._relPath != "") {

            bufferInfo._basePath += "->" + bufferInfo._relPath;
            bufferInfo._relPath = path;
            bufferInfo._depth += 1;
        }
        else
        {
            bufferInfo._relPath = path;
            bufferInfo._depth += 1;
        }
    }
}

//overload: get buffer inside buffer
content_t ArcSnp::GetContentOfArchive(BINFO& bufferInfo, DWORD depthLimit)
{
    bit7z::Bit7zLibrary lib{ "7z.dll" };
    content_t result;
    if (!bufferInfo._recursive)
    {
        bit7z::BitArchiveReader arc{ lib, bufferInfo._basePath, bit7z::BitFormat::Auto };
        const auto arc_items = arc.items();
        for (const auto& item : arc_items) {
            bit7z::buffer_t buffer{};
            arc.extractTo(buffer, item.index());
            result.emplace_back(DCOMP{
                                bufferInfo._basePath + "->" + bufferInfo._relPath,
                                std::string(item.path()),
                                PARENT_LEVEL_ARCHIVE,
                                item.size(),
                                buffer });
        }
    }
    else
    {
        bit7z::BitArchiveReader arc{ lib, bufferInfo._basePath, bit7z::BitFormat::Auto };
        auto arc_items = arc.items();
        for (auto& item : arc_items) {
            if (item.size() / 1024 <= MAX_BUFFER_SIZE)
            {   
                BINFO tmpInfo=bufferInfo;
                ArcSnp::GetBufferInfo(tmpInfo, item.path(), true);
                bit7z::buffer_t tmpBuffer{};
                arc.extractTo(tmpBuffer, item.index());
                auto output = ArcSnp::GetContentOfBuffer(tmpInfo, tmpBuffer);
                result.insert(result.end(), output.begin(), output.end());
            }
        }
    }
    return result;
}

content_t ArcSnp::GetContentOfBuffer(BINFO& bufferInfo, bit7z::buffer_t& buffer, DWORD depthLimit)
{
    bit7z::Bit7zLibrary lib{ "7z.dll" };
    content_t result;
    if (!bufferInfo._recursive)
    {
        bit7z::BitArchiveReader arc{ lib, buffer, bit7z::BitFormat::Auto };
        const auto arc_items = arc.items();
        for (const auto& item : arc_items) {
            bit7z::buffer_t tmpBuffer{};
            arc.extractTo(tmpBuffer, item.index());
            result.emplace_back(DCOMP{
                                bufferInfo._basePath + "->" + bufferInfo._relPath,
                                std::string(item.path()),
                                PARENT_LEVEL_ARCHIVE,
                                item.size(),
                                tmpBuffer });
        }
    }
    else
    {
        if (bufferInfo._depth + 1 <= DEPTH_LIMIT)
        {
            bit7z::BitArchiveReader arc{ lib, buffer, bit7z::BitFormat::Auto };
            auto arc_items = arc.items();
            for (auto& item : arc_items) {
                if (item.size() / 1024 <= MAX_BUFFER_SIZE)
                {
                    BINFO tmpInfo = bufferInfo;
                    ArcSnp::GetBufferInfo(tmpInfo, item.path(), true);
                    bit7z::buffer_t tmpBuffer{};
                    arc.extractTo(tmpBuffer, item.index());
                    auto output = ArcSnp::GetContentOfBuffer(tmpInfo, tmpBuffer);
                    result.insert(result.end(), output.begin(), output.end());
                }
            }
        }
        else
        {
            bufferInfo._recursive = false;
            bit7z::BitArchiveReader arc{ lib, buffer, bit7z::BitFormat::Auto };
            auto arc_items = arc.items();
            for (auto& item : arc_items) {
                if (item.size() / 1024 <= MAX_BUFFER_SIZE)
                {
                    BINFO tmpInfo = bufferInfo;
                    ArcSnp::GetBufferInfo(tmpInfo, item.path(), false);
                    bit7z::buffer_t tmpBuffer{};
                    arc.extractTo(tmpBuffer, item.index());
                    auto output = ArcSnp::GetContentOfBuffer(tmpInfo, tmpBuffer);
                    result.insert(result.end(), output.begin(), output.end());
                }
            }
        }
    }
    return result;
}

std::vector<std::string> ArcSnp::GetList(BINFO& bufferInfo, bit7z::buffer_t& buffer)
{
    return std::vector<std::string>();
}

DWORD ArcSnp::ClearBuffer(bit7z::buffer_t archBuffer)
{
    //TODO
    return 0;
}