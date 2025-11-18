#pragma once

#include <filesystem>
#include <string>
#include <variant>
#include <vector>
#include <cstdint>

enum class ArcSnpState {
    kExtractable = 0,
    kNotExist,
    kIsDirectory,
    kNotValid,
    kSolidFileSizeGreaterThanMax,
    kFileSizeGreaterThanMax,
    kBufferSizeGreaterThanMax,
    kStackFilled,
    kUnsafe
};

// FormatId - Exact values returned by BitInFormat::value() in bit7z
enum class FormatId : uint8_t
{
    kUnknown = 0x00,     // not a real format, used when detection fails
    kZip = 0x01,
    kBZip2 = 0x02,
    kRar = 0x03,
    kArj = 0x04,
    kZ = 0x05,
    kLzh = 0x06,
    kSevenZip = 0x07,    // This is what you get for real .7z files
    kCab = 0x08,
    kNsis = 0x09,
    kLzma = 0x0A,        // raw LZMA stream
    kLzma86 = 0x0B,
    kXz = 0x0C,
    kPpmd = 0x0D,
    kZstd = 0x0E,
    kVhdx = 0xC4,
    kCOFF = 0xC6,
    kExt = 0xC7,
    kVMDK = 0xC8,
    kVDI = 0xC9,
    kQCow = 0xCA,
    kGPT = 0xCB,
    kRar5 = 0xCC,        // RAR5 — most .rar files today return this or 0x03
    kIHex = 0xCD,
    kHxs = 0xCE,
    kTE = 0xCF,
    kUEFIc = 0xD0,
    kUEFIs = 0xD1,
    kSquashFS = 0xD2,
    kCramFS = 0xD3,
    kAPM = 0xD4,
    kMslz = 0xD5,
    kFlv = 0xD6,
    kSwf = 0xD7,
    kSwfc = 0xD8,
    kNtfs = 0xD9,
    kFat = 0xDA,
    kMbr = 0xDB,
    kVhd = 0xDC,
    kPe = 0xDD,          // Executables (also used for SFX detection sometimes)
    kElf = 0xDE,
    kMacho = 0xDF,
    kUdf = 0xE0,
    kXar = 0xE1,
    kMub = 0xE2,
    kHfs = 0xE3,
    kDmg = 0xE4,
    kCompound = 0xE5,    // .msi, old .doc/.xls/.ppt
    kWim = 0xE6,
    kIso = 0xE7,
    kChm = 0xE9,
    kSplit = 0xEA,
    kRpm = 0xEB,
    kDeb = 0xEC,
    kCpio = 0xED,
    kTar = 0xEE,
    kGZip = 0xEF,
};

struct ArcInfo {
    std::string base_path_{};
    size_t depth_{};
    ArcSnpState state_{};

    bool operator==(const ArcInfo& other) const {
        return (base_path_ == other.base_path_) && (depth_ == other.depth_);
    }
};
using ContentList = std::vector<ArcInfo>;

struct Decompressed {
    ArcInfo info;
    std::vector<unsigned char> buffer_{};
};
using Content = std::vector<Decompressed>;

struct Meta {
    uint32_t items_count_{};
    uint32_t folders_count_{};
    uint32_t files_count_{};
    uint64_t size_{};
    uint64_t pack_size_{};
    FormatId format_{ FormatId::kUnknown };

    /**
   * @brief Helper to convert the format_ member to a string extension.
   * @return A string representing the extension (e.g. ".7z"), or empty
   * string.
   */
    std::string GetExtension() const;
};

class ArcSnp {
public:
    ArcSnp(size_t file_size_limit, size_t solid_size_limit,
        size_t buffer_size_limit, size_t max_used_memory);
    ~ArcSnp();

    std::variant<Meta, ArcSnpState> GetMetadata(const std::string& file_path);

    /**********************************************************************
     * @brief	Retrieves the content from the specified path in single layer.
     *
     * @param	file_path : The path to the file (archive).
     * @return	a vector of Decompressed struct (Content).
     **********************************************************************/
    Content GetContent(const std::string& file_path);

    /**********************************************************************
     * @brief	Recursively retrieves the content from the specified path.
     *
     * @param	file_path : The path to the file (archive).
     * @param	depth_limit : The maximum recursion depth.
     * @return	a vector of Decompressed struct (Content).
     **********************************************************************/
    Content GetContent(const std::string& file_path, size_t depth_limit);

    ContentList GetContentList(const std::string& file_path);
    ContentList GetContentList(const std::string& file_path, size_t depth_limit);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};