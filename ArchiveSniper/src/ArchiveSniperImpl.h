#pragma once
#include "ArcSnp/ArchiveSniper.h" // Public API header
#include "ArcSnp/Shared7z.h"      // For bit7z library management
#include "checker.h"              // For IsZipBomb and ReadFile

// Add all required bit7z includes for the implementation
#include <bit7z/bitarchivereader.hpp> // For BitArchiveReader, BitArchiveItemInfo
#include <bit7z/bitexception.hpp>    // For BitException

/**
 * @struct ArcSnp::Impl
 * @brief Private implementation (Pimpl) for the ArcSnp class.
 *
 * This struct contains all the private members and methods for
 * archive inspection. It encapsulates the 7-Zip library, state management
 * (via RecursionContext), and all processing logic, keeping the
 * public ArcSnp header clean.
 */
struct ArcSnp::Impl {
    /**
     * @struct RecursionContext
     * @brief Holds the per-call state for a recursive archive operation.
     *
     * This object is passed down the call stack to track recursion depth
     * and total memory allocated for a single GetContent/GetContentList call,
     * making the ArcSnp::Impl class re-entrant and thread-safe.
     */
    struct RecursionContext {
        size_t current_depth_{ 0 };       ///< The current recursion depth.
        size_t depth_limit_{ 3 };         ///< The maximum depth to recurse to.
        bool recursion_enabled_{ true };  ///< Flag to stop recursion when limit is hit.
        double allocated_memory_mb_{
            0.0 };  ///< Total memory allocated in this context.
    };

public:
    /**
     * @brief Constructs the ArcSnp implementation.
     * @param file_size_limit Max file size in MB for the initial archive.
     * @param solid_size_limit Max file size in MB for solid archives.
     * @param buffer_size_limit Max size in MB for any single nested file.
     */
    Impl(size_t file_size_limit, size_t solid_size_limit,
        size_t buffer_size_limit, size_t max_used_memory);
    ~Impl();

    /**
     * @brief Gets metadata for the top-level archive.
     * @param file_path Path to the archive file.
     * @return A std::variant containing either a Meta struct or an ArcSnpState
     * error.
     */
    std::variant<Meta, ArcSnpState> GetMetadata(const std::string& file_path);

    /**
     * @brief Gets the content of an archive (non-recursive).
     * @param file_path Path to the archive file.
     * @return A Content vector (vector of Decompressed structs).
     */
    Content GetContent(const std::string& file_path);

    /**
     * @brief Gets the content of an archive recursively up to a depth limit.
     * @param file_path Path to the archive file.
     * @param depth_limit The maximum recursion depth.
     * @return A Content vector (vector of Decompressed structs).
     */
    Content GetContent(const std::string& file_path, size_t depth_limit);

    /**
     * @brief Lists the content of an archive (non-recursive).
     * @param file_path Path to the archive file.
     * @return A ContentList vector (vector of ArcInfo structs).
     */
    ContentList GetContentList(const std::string& file_path);

    /**
     * @brief Lists the content of an archive recursively up to a depth limit.
     * @param file_path Path to the archive file.
     * @param depth_limit The maximum recursion depth.
     * @return A ContentList vector (vector of ArcInfo structs).
     */
    ContentList GetContentList(const std::string& file_path, size_t depth_limit);

private:
    /**
     * @brief Recursively resolves a buffer to get file contents.
     * @param base_path The path of the parent item (e.g., "archive.zip->nested.rar").
     * @param buffer The buffer of the parent item to scan.
     * @param context The current recursion state (depth, memory).
     * @return A Content vector.
     */
    Content ResolveBuffer(const std::string& base_path,
        bit7z::buffer_t& buffer, RecursionContext& context);

    /**
     * @brief Recursively resolves a buffer to list file contents.
     * @param base_path The path of the parent item.
     * @param buffer The buffer of the parent item to scan.
     * @param context The current recursion state (depth, memory).
     * @return A ContentList vector.
     */
    ContentList ResolveListBuffer(const std::string& base_path,
        bit7z::buffer_t& buffer,
        RecursionContext& context);

    /**
     * @brief Checks a single item *within* an archive for safety (metadata only).
     * @param item_info The bit7z info struct for the item.
     * @param arc The archive reader (unused in optimized version).
     * @param context The current recursion context (for memory checks).
     * @return An ArcSnpState indicating the item's status.
     */
    [[nodiscard]] ArcSnpState CheckItem(
        const bit7z::BitArchiveItemInfo& item_info,
        const bit7z::BitArchiveReader& arc,
        const RecursionContext& context) const;

    /**
     * @brief Checks a top-level file for safety and validity.
     * @param file_path The path to the file.
     * @return An ArcSnpState indicating the file's status.
     */
    [[nodiscard]] ArcSnpState CheckFile(const std::string& file_path) const;

    // Private Members
    std::shared_ptr<bit7z::Bit7zLibrary> lib_;  ///< Shared pointer to the 7z lib.
    const size_t kMaxFileSize;  ///< Max size in MB for initial file.
    const size_t kMaxSolidSize;  ///< Max size in MB for solid archives.
    const size_t kMaxBufferSize;  ///< Max size in MB for nested files.
    const size_t kMaxUsedMemory;  ///< Max total memory in MB per call.
};