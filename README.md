# ArchiveSniper

**ArchiveSniper** is a lightweight, thread-safe C++ library for securely inspecting archive files (such as `.zip`, `.rar`, `.7z`, `.msi`, `.doc`, and more) purely in-memory.

It is designed for applications that need to scan or analyze the contents of user-provided archives without the risk and overhead of extracting them to disk. It provides a secure, in-memory buffer-based API that prevents common vulnerabilities like zip bombs and protects against excessive resource consumption.

## Key Features

* **In-Memory Processing:** All file inspection and extraction happens entirely in memory. No files are ever written to disk, eliminating disk I/O bottlenecks and temporary file artifacts.
* **Thread-Safe by Design:** The `ArcSnp` class is fully re-entrant. A single instance can be safely shared and used by multiple threads simultaneously to scan different archives.
* **Security First:** Built from the ground up to safely handle untrusted files.
* **Resource Limiting:** Enforces configurable limits on file sizes, memory usage, and buffer sizes.
* **Zip Bomb Detection:** Includes heuristics to detect and reject common zip bombs before they exhaust system memory.
* **Recursive Inspection:** Can scan archives nested within other archives up to a user-defined depth.
* **Simple, Modern C++ API:** Uses `std::variant` for clear success/error handling, `std::string` and `std::vector` for data, and a clean Pimpl-based header that is completely decoupled from the underlying bit7z library.
* **Wide Format Support:** Supports all archive formats recognized by the 7-Zip library.

## Requirements

* A C++17 (or newer) compiler.
* The $bit7z$ library (https://github.com/rikyoz/bit7z) (as a build dependency).
* The 7-Zip library (7zip.dll) available at runtime.

## Building the Project

This project is built using CMake.

```bash
# Clone the repository (recursive to fetch submodules)
git clone --recursive https://github.com/moligarch/ArchiveSniper.git
cd ArchiveSniper

# Build bit7z with automatic format detection (required for static libs)
powershell ./build_bit7z.ps1
```

You will need to ensure CMake can find your `bit7z` library dependency.

## Quick Start / Usage

The public API is minimal and easy to use. The most critical step is initializing the 7-Zip library **once** at application startup.

```cpp
#include "ArcSnp/ArchiveSniper.h"
#include "ArcSnp/Shared7z.h" // Required for initialization
#include <iostream>
#include <variant>

int main() {
try {
// --- 1. Initialize the Library (Once) ---
// This must be called once before creating any ArcSnp objects.
// It points to the 7-Zip DLL/SO file.
Shared7z::Init("7zip.dll");

    // --- 2. Create an Inspector ---
    // Configure the security limits (in MB):
    // 1. Max File Size (100 MB)
    // 2. Max Solid File Size (50 MB)
    // 3. Max Single Buffer Size (20 MB)
    // 4. Max Total Memory per Call (500 MB)
    ArcSnp sniper(100, 50, 20, 500);

    // --- 3. Get Archive Metadata ---
    std::string archive_path = "C:\\path\\to\\archive.zip";
    std::variant<Meta, ArcSnpState> result = sniper.GetMetadata(archive_path);

    if (std::holds_alternative<Meta>(result)) {
        const Meta& metadata = std::get<Meta>(result);
        
        std::cout << "Archive Format: " << metadata.GetExtension() << std::endl;
        std::cout << "File Count: " << metadata.files_count_ << std::endl;
        std::cout << "Total Size: " << metadata.size_ << " bytes" << std::endl;

    } else {
        ArcSnpState error = std::get<ArcSnpState>(result);
        std::cerr << "Failed to get metadata: " << static_cast<int>(error) << std::endl;
    }

    // --- 4. Get Recursive Content (up to 3 levels deep) ---
    Content files = sniper.GetContent(archive_path, 3);

    for (const auto& file : files) {
        std::cout << "Path: " << file.info.base_path_ << "\n";
        std::cout << "  - Depth: " << file.info.depth_ << "\n";
        std::cout << "  - State: " << static_cast<int>(file.info.state_) << "\n";
        std::cout << "  - Size: " << file.buffer_.size() << " bytes\n";

        if (file.info.state_ == ArcSnpState::kExtractable || 
            file.info.state_ == ArcSnpState::kNotValid) {
            // You can now safely use the file.buffer_
            // (e.g., run a virus scan on it)
        }
    }

} catch (const std::exception& e) {
    std::cerr << "An error occurred: " << e.what() << std::endl;
    return 1;
}

return 0;


}
```

## API Overview

### `Shared7z::Init(const fs::path& library_path)`

A static method that **must** be called once to load the 7-Zip DLL/SO into memory.

### `ArcSnp(size_t file_size_limit, size_t solid_size_limit, size_t buffer_size_limit, size_t max_used_memory)`

The main class constructor. All size limits are specified in **Megabytes (MB)**.

* `file_size_limit`: The maximum size of the initial archive file to open.
* `solid_size_limit`: A (usually stricter) size limit for *solid* archives.
* `buffer_size_limit`: The maximum size for any *single nested file* extracted into a buffer.
* `max_used_memory`: The maximum *total memory* that one `GetContent` or `GetContentList` call can allocate. This is the primary defense against zip bombs.

### `std::variant<Meta, ArcSnpState> GetMetadata(const std::string& file_path)`

Returns metadata for the top-level archive.

* On success, returns a `Meta` struct.
* On failure (e.g., file not found, is not an archive), returns an `ArcSnpState` error.

### `struct Meta`

Contains basic archive metadata.

* `uint32_t items_count_`
* `uint32_t folders_count_`
* `uint32_t files_count_`
* `uint64_t size_`
* `uint64_t pack_size_`
* `FormatId format_`: A type-safe enum for the archive format.
* `std::string GetFormatExtension() const`: A helper function that returns a string representation of the format (e.g., "zip", "rar", "compound").

### `Content GetContent(const std::string& file_path, size_t depth_limit)`

Returns a `std::vector<Decompressed>` containing the buffers for all files found up to the specified `depth_limit`.

* `Content` is `std::vector<Decompressed>`.
* `Decompressed` contains an `ArcInfo` struct and a `std::vector<unsigned char> buffer_`.

### `ContentList GetContentList(const std::string& file_path, size_t depth_limit)`

A faster alternative to `GetContent` that does not retain the file buffers. It returns a `std::vector<ArcInfo>`.

* `ContentList` is std::vector<ArcInfo>.
* `ArcInfo` contains the `base_path_`, `depth_`, and `state_` for a file.

### `enum class ArcSnpState`

Indicates the status of a file or archive. Key values include:

* `kExtractable`: The file is a valid archive and can be recursed into.
* `kNotValid`: The file is not an archive (e.g., a .txt file) but is otherwise safe.
* `kUnsafe`: The file was flagged as a zip bomb.
* `kStackFilled`: The max_used_memory limit was hit.
* `kBufferSizeGreaterThanMax`: A single file exceeded the buffer_size_limit.
* ...and other error states.

### `enum class FormatId`

A type-safe enum representing all formats supported by 7-Zip. See `ArchiveSniper.h` for the complete list (e.g., `FormatId::kZip`, `FormatId::kRar5`, `FormatId::kCompound`).

## Concurrency

The `ArcSnp` class is fully re-entrant and thread-safe. A single, shared `ArcSnp` instance can be used by any number of threads to process different files concurrently. All per-call state (like memory usage and recursion depth) is managed in a private `RecursionContext` struct on the stack, preventing data races.

## License

This project is licensed under the MIT License.