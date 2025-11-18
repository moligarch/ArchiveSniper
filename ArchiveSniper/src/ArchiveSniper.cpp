#include "ArchiveSniperImpl.h" // Must be first

#include <filesystem>
#include <string>
#include <vector>

#include "ArcSnp/ArchiveSniper.h"
#include "checker.h"

namespace fs = std::filesystem;

// =======================================================================
// === ArcSnp Public API Forwarders
// =======================================================================

ArcSnp::ArcSnp(size_t file_size_limit, size_t solid_size_limit,
    size_t buffer_size_limit, size_t max_used_memory) {
    impl_ = std::make_unique<Impl>(file_size_limit, solid_size_limit,
        buffer_size_limit, max_used_memory);
}

ArcSnp::~ArcSnp() {}

std::string Meta::GetExtension() const {
    switch (format_) {
        case FormatId::kZip: return "zip";
        case FormatId::kBZip2: return "bz2";
        case FormatId::kRar: return "rar";
        case FormatId::kArj: return "arj";
        case FormatId::kZ: return "z";
        case FormatId::kLzh: return "lzh";
        case FormatId::kSevenZip: return "7z";
        case FormatId::kCab: return "cab";
        case FormatId::kNsis: return "nsis";  // Descriptive name
        case FormatId::kLzma: return "lzma";
        case FormatId::kLzma86: return "lzma86";
        case FormatId::kXz: return "xz";
        case FormatId::kPpmd: return "ppmd";
        case FormatId::kZstd: return "zst";
        case FormatId::kVhdx: return "vhdx";
        case FormatId::kCOFF: return "coff";  // Descriptive name
        case FormatId::kExt: return "ext";
        case FormatId::kVMDK: return "vmdk";
        case FormatId::kVDI: return "vdi";
        case FormatId::kQCow: return "qcow";
        case FormatId::kGPT: return "gpt";  // Descriptive name
        case FormatId::kRar5: return "rar";
        case FormatId::kIHex: return "hex";
        case FormatId::kHxs: return "hxs";
        case FormatId::kTE: return "te";
        case FormatId::kUEFIc: return "uefic";
        case FormatId::kUEFIs: return "uefis";
        case FormatId::kSquashFS: return "squashfs";
        case FormatId::kCramFS: return "cramfs";
        case FormatId::kAPM: return "apm";
        case FormatId::kMslz: return "mslz";
        case FormatId::kFlv: return "flv";
        case FormatId::kSwf: return "swf";
        case FormatId::kSwfc: return "swfc";
        case FormatId::kNtfs: return "ntfs";  // Descriptive name
        case FormatId::kFat: return "fat";    // Descriptive name
        case FormatId::kMbr: return "mbr";
        case FormatId::kVhd: return "vhd";
        case FormatId::kPe: return "pe";  // Descriptive name
        case FormatId::kElf: return "elf";    // Descriptive name
        case FormatId::kMacho: return "macho";  // Descriptive name
        case FormatId::kUdf: return "udf";    // Descriptive name
        case FormatId::kXar: return "xar";
        case FormatId::kMub: return "mub";
        case FormatId::kHfs: return "hfs";
        case FormatId::kDmg: return "dmg";
        case FormatId::kCompound: return "compound";  // Descriptive name
        case FormatId::kWim: return "wim";
        case FormatId::kIso: return "iso";
        case FormatId::kChm: return "chm";
        case FormatId::kSplit: return "split";  // Descriptive name
        case FormatId::kRpm: return "rpm";
        case FormatId::kDeb: return "deb";
        case FormatId::kCpio: return "cpio";
        case FormatId::kTar: return "tar";
        case FormatId::kGZip: return "gz";
        default: return "";
    }
}

std::variant<Meta, ArcSnpState> ArcSnp::GetMetadata(
    const std::string& file_path) {
    return impl_->GetMetadata(file_path);
}

Content ArcSnp::GetContent(const std::string& file_path) {
    return impl_->GetContent(file_path);
}

Content ArcSnp::GetContent(const std::string& file_path, size_t depth_limit) {
    return impl_->GetContent(file_path, depth_limit);
}

ContentList ArcSnp::GetContentList(const std::string& file_path) {
    return impl_->GetContentList(file_path);
}

ContentList ArcSnp::GetContentList(const std::string& file_path,
    size_t depth_limit) {
    return impl_->GetContentList(file_path, depth_limit);
}

// =======================================================================
// === ArcSnp::Impl Method Implementations
// =======================================================================

ArcSnp::Impl::Impl(size_t file_size_limit, size_t solid_size_limit,
    size_t buffer_size_limit, size_t max_used_memory)
    : kMaxFileSize(file_size_limit),
    kMaxSolidSize(solid_size_limit),
    kMaxBufferSize(buffer_size_limit),
    kMaxUsedMemory(max_used_memory){
    lib_ = Shared7z::GetLibrary();
}

ArcSnp::Impl::~Impl() {}

std::variant<Meta, ArcSnpState> ArcSnp::Impl::GetMetadata(
    const std::string& file_path) {
    const auto file_state = CheckFile(file_path);
    if (file_state != ArcSnpState::kExtractable) {
        return file_state;
    }

    bit7z::BitArchiveReader arc{ *lib_, file_path, bit7z::BitFormat::Auto };
    return Meta{
        .items_count_ = arc.itemsCount(),
        .folders_count_ = arc.foldersCount(),
        .files_count_ = arc.filesCount(),
        .size_ = arc.size(),
        .pack_size_ = arc.packSize(),
        .format_ = static_cast<FormatId>(arc.detectedFormat().value())
    };
}

Content ArcSnp::Impl::GetContent(const std::string& file_path) {
    RecursionContext context;
    context.depth_limit_ = 1;
    context.current_depth_ = 0;

    Content result;
    const auto file_state = CheckFile(file_path);

    if (file_state != ArcSnpState::kExtractable) {
        result.emplace_back(
            Decompressed{ {.base_path_ = file_path,
                          .depth_ = context.current_depth_,
                          .state_ = file_state},
                         {} });
        return result;
    }

    bit7z::BitArchiveReader arc{ *lib_, file_path, bit7z::BitFormat::Auto };
    auto arc_items = arc.items();
    for (const auto& item : arc_items) {
        const auto item_path{ file_path + "->" + item.path() };
        const auto item_state = CheckItem(item, arc, context);

        if (item_state == ArcSnpState::kUnsafe ||
            item_state == ArcSnpState::kStackFilled) {
            result.emplace_back(
                Decompressed{ {.base_path_ = item_path,
                              .depth_ = context.current_depth_ + 1,
                              .state_ = item_state},
                             {} });
            return result;
        }

        if (item_state != ArcSnpState::kNotExist &&
            item_state != ArcSnpState::kIsDirectory) {
            auto tmp_buffer = std::make_unique<bit7z::buffer_t>(item.size());
            arc.extractTo(*tmp_buffer, item.index());
            context.allocated_memory_mb_ +=
                (*tmp_buffer).size() / (1024.0 * 1024.0);

            if (Checker::IsZipBomb(*tmp_buffer)) {
                result.emplace_back(
                    Decompressed{ {.base_path_ = item_path,
                                  .depth_ = context.current_depth_ + 1,
                                  .state_ = ArcSnpState::kUnsafe},
                                 {} });
                context.allocated_memory_mb_ -=
                    (*tmp_buffer).size() / (1024.0 * 1024.0);
                return result;
            }

            result.emplace_back(
                Decompressed{ {.base_path_ = item_path,
                              .depth_ = context.current_depth_ + 1,
                              .state_ = item_state},
                             std::move(*tmp_buffer) });  // <-- std::move
        }
    }
    return result;
}

Content ArcSnp::Impl::GetContent(const std::string& file_path,
    size_t depth_limit) {
    RecursionContext context;
    context.depth_limit_ = depth_limit;

    Content result;
    const auto file_state = CheckFile(file_path);
    if (file_state != ArcSnpState::kExtractable) {
        result.emplace_back(
            Decompressed{ {.base_path_ = file_path,
                          .depth_ = context.current_depth_,
                          .state_ = file_state},
                         {} });
        return result;
    }

    bit7z::BitArchiveReader arc{ *lib_, file_path, bit7z::BitFormat::Auto };
    auto arc_items = arc.items();

    for (const auto& item : arc_items) {
        const auto item_path{ file_path + "->" + item.path() };
        const auto item_state = CheckItem(item, arc, context);

        if (item_state == ArcSnpState::kUnsafe ||
            item_state == ArcSnpState::kStackFilled) {
            result.emplace_back(
                Decompressed{ {.base_path_ = item_path,
                              .depth_ = context.current_depth_ + 1,
                              .state_ = item_state},
                             {} });
            return result;
        }

        if (item_state == ArcSnpState::kExtractable) {
            auto tmp_buffer = std::make_unique<bit7z::buffer_t>(item.size());
            arc.extractTo(*tmp_buffer, item.index());
            context.allocated_memory_mb_ +=
                (*tmp_buffer).size() / (1024.0 * 1024.0);

            if (Checker::IsZipBomb(*tmp_buffer)) {
                result.emplace_back(
                    Decompressed{ {.base_path_ = item_path,
                                  .depth_ = context.current_depth_ + 1,
                                  .state_ = ArcSnpState::kUnsafe},
                                 {} });
                context.allocated_memory_mb_ -=
                    (*tmp_buffer).size() / (1024.0 * 1024.0);
                return result;
            }

            try {
                bit7z::BitArchiveReader tmp_arc{ *lib_, *tmp_buffer,
                                                bit7z::BitFormat::Auto };
                tmp_arc.test();

                auto output = ResolveBuffer(item_path, *tmp_buffer, context);
                result.insert(result.end(), output.begin(), output.end());
            }
            catch (const bit7z::BitException&) {
                result.emplace_back(
                    Decompressed{ {.base_path_ = item_path,
                                  .depth_ = context.current_depth_ + 1,
                                  .state_ = ArcSnpState::kNotValid},
                                 std::move(*tmp_buffer) });  // <-- std::move
            }
            context.allocated_memory_mb_ -=
                (*tmp_buffer).size() / (1024.0 * 1024.0);
        }
        else if (item_state != ArcSnpState::kNotExist &&
            item_state != ArcSnpState::kIsDirectory) {
            auto tmp_buffer = std::make_unique<bit7z::buffer_t>(item.size());
            arc.extractTo(*tmp_buffer, item.index());
            result.emplace_back(
                Decompressed{ {.base_path_ = item_path,
                              .depth_ = context.current_depth_ + 1,
                              .state_ = item_state},
                             std::move(*tmp_buffer) });  // <-- std::move
            context.allocated_memory_mb_ +=
                (*tmp_buffer).size() / (1024.0 * 1024.0);
        }
    }
    return result;
}

ContentList ArcSnp::Impl::GetContentList(const std::string& file_path) {
    RecursionContext context;
    context.depth_limit_ = 1;
    context.current_depth_ = 0;

    ContentList result;
    const auto file_state = CheckFile(file_path);
    if (file_state != ArcSnpState::kExtractable) {
        result.emplace_back(ArcInfo{ .base_path_ = file_path,
                                    .depth_ = context.current_depth_,
                                    .state_ = file_state });
        return result;
    }

    bit7z::BitArchiveReader arc{ *lib_, file_path, bit7z::BitFormat::Auto };
    auto arc_items = arc.items();

    for (const auto& item : arc_items) {
        const auto item_path{ file_path + "->" + item.path() };
        const auto item_state = CheckItem(item, arc, context);
        result.emplace_back(ArcInfo{ .base_path_ = item_path,
                                    .depth_ = context.current_depth_ + 1,
                                    .state_ = item_state });
    }
    return result;
}

ContentList ArcSnp::Impl::GetContentList(const std::string& file_path,
    size_t depth_limit) {
    RecursionContext context;
    context.depth_limit_ = depth_limit;

    ContentList result;
    const auto file_state = CheckFile(file_path);
    if (file_state != ArcSnpState::kExtractable) {
        result.emplace_back(ArcInfo{ .base_path_ = file_path,
                                    .depth_ = context.current_depth_,
                                    .state_ = file_state });
        return result;
    }

    bit7z::BitArchiveReader arc{ *lib_, file_path, bit7z::BitFormat::Auto };
    auto arc_items = arc.items();

    for (const auto& item : arc_items) {
        const auto item_path{ file_path + "->" + item.path() };
        const auto item_state = CheckItem(item, arc, context);

        if (item_state == ArcSnpState::kUnsafe ||
            item_state == ArcSnpState::kStackFilled) {
            result.emplace_back(ArcInfo{ .base_path_ = item_path,
                                        .depth_ = context.current_depth_ + 1,
                                        .state_ = item_state });
            return result;
        }

        if (item_state == ArcSnpState::kExtractable) {
            auto tmp_buffer = std::make_unique<bit7z::buffer_t>(item.size());
            arc.extractTo(*tmp_buffer, item.index());
            context.allocated_memory_mb_ +=
                (*tmp_buffer).size() / (1024.0 * 1024.0);

            if (Checker::IsZipBomb(*tmp_buffer)) {
                result.emplace_back(ArcInfo{ .base_path_ = item_path,
                                            .depth_ = context.current_depth_ + 1,
                                            .state_ = ArcSnpState::kUnsafe });
                context.allocated_memory_mb_ -=
                    (*tmp_buffer).size() / (1024.0 * 1024.0);
                return result;
            }

            try {
                bit7z::BitArchiveReader tmp_arc{ *lib_, *tmp_buffer,
                                                bit7z::BitFormat::Auto };
                tmp_arc.test();

                auto output = ResolveListBuffer(item_path, *tmp_buffer, context);
                result.insert(result.end(), output.begin(), output.end());
            }
            catch (const bit7z::BitException&) {
                result.emplace_back(ArcInfo{ .base_path_ = item_path,
                                            .depth_ = context.current_depth_ + 1,
                                            .state_ = ArcSnpState::kNotValid });
            }
            context.allocated_memory_mb_ -=
                (*tmp_buffer).size() / (1024.0 * 1024.0);
        }
        else if (item_state != ArcSnpState::kNotExist &&
            item_state != ArcSnpState::kIsDirectory) {
            auto tmp_buffer = std::make_unique<bit7z::buffer_t>(item.size());
            arc.extractTo(*tmp_buffer, item.index());
            context.allocated_memory_mb_ +=
                (*tmp_buffer).size() / (1024.0 * 1024.0);
            result.emplace_back(ArcInfo{ .base_path_ = item_path,
                                        .depth_ = context.current_depth_ + 1,
                                        .state_ = item_state });
            context.allocated_memory_mb_ -=
                (*tmp_buffer).size() / (1024.0 * 1024.0);
        }
    }
    return result;
}

Content ArcSnp::Impl::ResolveBuffer(const std::string& base_path,
    bit7z::buffer_t& buffer,
    RecursionContext& context) {
    if (++context.current_depth_ >= context.depth_limit_) {
        context.current_depth_--;  // Fix stack
        return {};
    }

    bit7z::BitArchiveReader arc{ *lib_, buffer, bit7z::BitFormat::Auto };
    auto arc_items = arc.items();
    Content result;

    for (const auto& item : arc_items) {
        const auto item_path{ base_path + "->" + item.path() };
        const auto item_state = CheckItem(item, arc, context);

        if (item_state == ArcSnpState::kUnsafe ||
            item_state == ArcSnpState::kStackFilled) {
            result.emplace_back(
                Decompressed{ {.base_path_ = item_path,
                              .depth_ = context.current_depth_ + 1,
                              .state_ = item_state},
                             {} });
            context.current_depth_--;
            return result;
        }

        if (item_state == ArcSnpState::kExtractable) {
            auto tmp_buffer = std::make_unique<bit7z::buffer_t>(item.size());
            arc.extractTo(*tmp_buffer, item.index());
            context.allocated_memory_mb_ +=
                (*tmp_buffer).size() / (1024.0 * 1024.0);

            if (Checker::IsZipBomb(*tmp_buffer)) {
                result.emplace_back(
                    Decompressed{ {.base_path_ = item_path,
                                  .depth_ = context.current_depth_ + 1,
                                  .state_ = ArcSnpState::kUnsafe},
                                 {} });
                context.allocated_memory_mb_ -=
                    (*tmp_buffer).size() / (1024.0 * 1024.0);
                context.current_depth_--;
                return result;
            }

            Content nested_output;
            bool is_archive = false;
            try {
                bit7z::BitArchiveReader tmp_arc{ *lib_, *tmp_buffer,
                                                bit7z::BitFormat::Auto };
                tmp_arc.test();
                is_archive = true;

                nested_output = ResolveBuffer(item_path, *tmp_buffer, context);
                result.insert(result.end(), nested_output.begin(), nested_output.end());
            }
            catch (const bit7z::BitException&) {
                is_archive = false;
            }

            if (!is_archive || nested_output.empty()) {
                result.emplace_back(
                    Decompressed{ {.base_path_ = item_path,
                                  .depth_ = context.current_depth_ + 1,
                                  .state_ = is_archive ? item_state
                                                       : ArcSnpState::kNotValid},
                                 std::move(*tmp_buffer) });  // <-- std::move
            }

            context.allocated_memory_mb_ -=
                (*tmp_buffer).size() / (1024.0 * 1024.0);
        }
        else if (item_state != ArcSnpState::kNotExist &&
            item_state != ArcSnpState::kIsDirectory) {
            auto tmp_buffer = std::make_unique<bit7z::buffer_t>(item.size());
            arc.extractTo(*tmp_buffer, item.index());
            result.emplace_back(
                Decompressed{ {.base_path_ = item_path,
                              .depth_ = context.current_depth_ + 1,
                              .state_ = item_state},
                             std::move(*tmp_buffer) });  // <-- std::move
            context.allocated_memory_mb_ +=
                (*tmp_buffer).size() / (1024.0 * 1024.0);
        }
    }
    context.current_depth_--;
    return result;
}

ContentList ArcSnp::Impl::ResolveListBuffer(const std::string& base_path,
    bit7z::buffer_t& buffer,
    RecursionContext& context) {
    if (++context.current_depth_ >= context.depth_limit_) {
        context.current_depth_--;  // Fix stack
        return {};
    }

    bit7z::BitArchiveReader arc{ *lib_, buffer, bit7z::BitFormat::Auto };
    auto arc_items = arc.items();
    ContentList result;

    for (const auto& item : arc_items) {
        const auto item_path{ base_path + "->" + item.path() };
        const auto item_state = CheckItem(item, arc, context);

        if (item_state == ArcSnpState::kUnsafe ||
            item_state == ArcSnpState::kStackFilled) {
            result.emplace_back(ArcInfo{ .base_path_ = item_path,
                                        .depth_ = context.current_depth_ + 1,
                                        .state_ = item_state });
            context.current_depth_--;
            return result;
        }

        if (item_state == ArcSnpState::kExtractable) {
            auto tmp_buffer = std::make_unique<bit7z::buffer_t>(item.size());
            arc.extractTo(*tmp_buffer, item.index());
            context.allocated_memory_mb_ +=
                (*tmp_buffer).size() / (1024.0 * 1024.0);

            if (Checker::IsZipBomb(*tmp_buffer)) {
                result.emplace_back(ArcInfo{ .base_path_ = item_path,
                                            .depth_ = context.current_depth_ + 1,
                                            .state_ = ArcSnpState::kUnsafe });
                context.allocated_memory_mb_ -=
                    (*tmp_buffer).size() / (1024.0 * 1024.0);
                context.current_depth_--;
                return result;
            }

            bool is_archive = false;
            try {
                bit7z::BitArchiveReader tmp_arc{ *lib_, *tmp_buffer,
                                                bit7z::BitFormat::Auto };
                tmp_arc.test();
                is_archive = true;

                auto output = ResolveListBuffer(item_path, *tmp_buffer, context);
                result.insert(result.end(), output.begin(), output.end());
            }
            catch (const bit7z::BitException&) {
                is_archive = false;
            }

            result.emplace_back(
                ArcInfo{ .base_path_ = item_path,
                        .depth_ = context.current_depth_ + 1,
                        .state_ = is_archive ? item_state : ArcSnpState::kNotValid });

            context.allocated_memory_mb_ -=
                (*tmp_buffer).size() / (1024.0 * 1024.0);
        }
        else if (item_state != ArcSnpState::kNotExist &&
            item_state != ArcSnpState::kIsDirectory) {
            result.emplace_back(ArcInfo{ .base_path_ = item_path,
                                        .depth_ = context.current_depth_ + 1,
                                        .state_ = item_state });
        }
    }
    context.current_depth_--;
    return result;
}

[[nodiscard]] ArcSnpState ArcSnp::Impl::CheckItem(
    const bit7z::BitArchiveItemInfo& item_info,
    const bit7z::BitArchiveReader& arc,
    const RecursionContext& context) const {
    auto buffer_size_mb = item_info.size() / (1024.0 * 1024.0);
    if (buffer_size_mb + context.allocated_memory_mb_ > kMaxUsedMemory) {
        return ArcSnpState::kStackFilled;
    }
    if (buffer_size_mb > kMaxBufferSize) {
        return ArcSnpState::kBufferSizeGreaterThanMax;
    }

    if (item_info.isDir()) {
        return ArcSnpState::kIsDirectory;
    }

    // Optimistic check. Real checks (bomb, archive test) happen post-extraction.
    return ArcSnpState::kExtractable;
}

[[nodiscard]] ArcSnpState ArcSnp::Impl::CheckFile(
    const std::string& file_path) const {
    fs::path path{ file_path };
    if (path.is_relative()) {
        path = std::filesystem::absolute(path);
    }

    if (!std::filesystem::exists(path)) {
        return ArcSnpState::kNotExist;
    }

    if (std::filesystem::is_directory(path)) {
        return ArcSnpState::kIsDirectory;
    }

    double file_size_mb = std::filesystem::file_size(path) / (1024.0 * 1024.0);
    if (file_size_mb > kMaxFileSize) {
        return ArcSnpState::kFileSizeGreaterThanMax;
    }

    auto file_buffer = Checker::ReadFile(path);
    if (Checker::IsZipBomb(file_buffer)) {
        return ArcSnpState::kUnsafe;
    }

    try {
        bit7z::BitArchiveReader tmp_arc{ *lib_, file_path, bit7z::BitFormat::Auto };
        tmp_arc.test();
        if (tmp_arc.isSolid() && file_size_mb > kMaxSolidSize) {
            return ArcSnpState::kSolidFileSizeGreaterThanMax;
        }
        else {
            return ArcSnpState::kExtractable;
        }
    }
    catch (const bit7z::BitException&) {
        return ArcSnpState::kNotValid;
    }
}