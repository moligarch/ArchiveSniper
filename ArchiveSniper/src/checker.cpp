#include "checker.h"

#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <vector>

namespace Checker {

    /**
     * @brief Heuristically checks if a buffer is a zip bomb.
     *
     * This check is based on character frequency. A buffer with a very
     * low number of unique characters (e.g., 3 or less) making up the
     * majority of the file is flagged as a potential bomb.
     *
     * @param buffer The file content to check.
     * @return true if the buffer is suspected to be a zip bomb, false otherwise.
     */
    bool IsZipBomb(const std::vector<unsigned char>& buffer) {
        // specific character -> share of it from all of content
        std::unordered_map<unsigned char, double_t> freq_percent_map;

        // Avoid division by zero if the buffer is empty
        if (buffer.empty()) {
            return false;
        }

        // Calculate the frequency of each character in the buffer
        for (const auto b : buffer) {
            freq_percent_map[b] += (100.0 / buffer.size());
        }

        // Find the character with the highest frequency
        double_t max_frequency_percent = 0;
        for (const auto& entry : freq_percent_map) {
            if (entry.second > max_frequency_percent) {
                max_frequency_percent = entry.second;
            }
        }

        // Check if the dominant character constitutes most of the buffer
        auto threshold = freq_percent_map.size() <= 3
            ? 100.0 / freq_percent_map.size()
            : 80;  // Adjust the threshold as needed
        if (max_frequency_percent >= threshold && buffer.size() > 1024) {
            return true;
        }
        return false;
    }

    /**
     * @brief Reads the entire content of a file into a byte vector.
     *
     * @param file_path The path to the file to read.
     * @return A std::vector<unsigned char> containing the file's content,
     * or an empty vector if the file could not be opened.
     */
    std::vector<unsigned char> ReadFile(const fs::path& file_path) {
        std::ifstream file_stream(file_path, std::ios::binary);
        if (!file_stream) {
            return std::vector<unsigned char>();  // Return an empty buffer
        }
        // Determine the file size
        file_stream.seekg(0, std::ios::end);
        std::streampos file_size = file_stream.tellg();
        file_stream.seekg(0, std::ios::beg);
        // Read the file into the buffer
        std::vector<unsigned char> buffer(file_size);
        file_stream.read(reinterpret_cast<char*>(buffer.data()), file_size);
        file_stream.close();
        return buffer;
    }

}  // namespace Checker