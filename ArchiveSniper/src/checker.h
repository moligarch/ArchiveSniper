#pragma once

#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

namespace Checker {

	bool IsZipBomb(const std::vector<unsigned char>& buffer);
	std::vector<unsigned char> ReadFile(const fs::path& file_path);

}  // namespace Checker