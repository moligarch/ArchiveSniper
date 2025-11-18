#pragma once

#include <bit7z/bit7zlibrary.hpp>
#include <filesystem>
#include <memory>
#include <mutex>

namespace fs = std::filesystem;

/**
 * @class Shared7z
 * @brief Manages the singleton instance of the bit7z::Bit7zLibrary.
 */
class Shared7z {
public:
	// Delete copy/move constructors and assignment operators
	Shared7z(const Shared7z&) = delete;
	Shared7z& operator=(const Shared7z&) = delete;
	Shared7z(Shared7z&&) = delete;
	Shared7z& operator=(Shared7z&&) = delete;

	/**
	 * @brief Initializes the singleton instance with the path to 7zip.dll.
	 *
	 * @param library_path The full path to the 7zip.dll (or .so).
	 * @throws bit7z::BitException if the library cannot be loaded.
	 */
	static void Init(const fs::path& library_path);

	/**
	 * @brief Gets the shared pointer to the 7z library.
	 *
	 * @return A std::shared_ptr<bit7z::Bit7zLibrary>.
	 * @throws std::runtime_error if Init() has not been called successfully.
	 */
	static std::shared_ptr<bit7z::Bit7zLibrary> GetLibrary();

private:
	/**
	 * @brief Private constructor.
	 */
	Shared7z();

	/**
	 * @brief The internal initialization logic.
	 * @param library_path Path to the 7z DLL.
	 */
	void Initialize(const fs::path& library_path);

	/**
	 * @brief Gets the singleton instance. (Private helper)
	 */
	static Shared7z& GetInstance();

	// Static members for the singleton pattern
	static std::unique_ptr<Shared7z> instance_;
	static std::once_flag init_flag_;

	// Member variable holding the library
	std::shared_ptr<bit7z::Bit7zLibrary> lib_;
};