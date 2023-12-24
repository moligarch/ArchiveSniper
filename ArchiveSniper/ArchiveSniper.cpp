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


DWORD ArcSnp::GetArchive(ARCH& archive, const std::string& path)
{
    return 0;
}

bit7z::buffer_t ArcSnp::GetBuffer(ARCH& archive)
{
    return bit7z::buffer_t();
}

std::map<const std::string, bit7z::buffer_t> ArcSnp::GetContent(bit7z::buffer_t archBuffer)
{
    return std::map<const std::string, bit7z::buffer_t>();
}

DWORD ArcSnp::ClearBuffer(bit7z::buffer_t archBuffer)
{
    //TODO
    return 0;
}

arch_t ArcSnp::GetBufferR(ARCH& archive, const DWORD dwLevel)
{
    return arch_t();
}

content_t ArcSnp::GetContentR(bit7z::buffer_t archBuffer, const DWORD dwLevel)
{
    return content_t();
}
