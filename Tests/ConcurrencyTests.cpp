#include "pch.h"

namespace ConcurrencyTest {
    TEST_CLASS(ConcurrencyTests) {
public:
    TEST_METHOD(ParallelGetContent) {
        // Create ONE reader. This is the key to a re-entrancy test.
        // We give it generous limits.
        ArcSnp reader(100, 100, 100, 2000);

        // This atomic flag will be set to false if any thread fails
        std::atomic_bool all_threads_passed = true;

        // Define a lambda for one unit of work
        auto task = [&](const std::string& path,
            const std::string& expected_file,
            const std::string& expected_content) {
                try {
                    Content result = reader.GetContent(path, 5); // Use depth 5
                    auto iter = std::find_if(result.begin(), result.end(),
                        [&](const Decompressed& item) {
                            return item.info.base_path_ ==
                                path + "->" + expected_file;
                        });

                    if (iter == result.end()) {
                        Logger::WriteMessage(
                            ("FAILURE: Could not find " + expected_file + " in " + path)
                            .c_str());
                        all_threads_passed = false;
                        return;
                    }

                    std::string buffer(iter->buffer_.begin(), iter->buffer_.end());
                    if (buffer != expected_content) {
                        Logger::WriteMessage(
                            ("FAILURE: Content mismatch in " + expected_file).c_str());
                        all_threads_passed = false;
                    }
                }
                catch (const std::exception& e) {
                    Logger::WriteMessage(
                        ("FAILURE: Exception in thread for " + path + ": " + e.what())
                        .c_str());
                    all_threads_passed = false;
                }
            };

        // --- Launch parallel tasks ---
        // We use std::async to run tasks in parallel.
        // We will use test files from the other unit tests.

        auto future1 = std::async(
            std::launch::async, task,
            "..\\..\\..\\..\\Tests\\unit\\test.docx",
            "test.txt", "Find This For Test");

        auto future2 = std::async(
            std::launch::async, task,
            "..\\..\\..\\..\\Tests\\unit\\depth2.zip",
            "depth1.zip->test.txt", "Find This For Test");

        auto future3 = std::async(
            std::launch::async, task,
            "..\\..\\..\\..\\Tests\\performance\\recursion\\depth3.zip",
            "depth2.zip->depth1.zip->test.txt", "Find This For Test");

        // A better task for task 4 (solid archive test)
        auto task_solid = [&]() {
            try {
                std::string path = "..\\..\\..\\..\\Tests\\unit\\solid.rar";
                // Use a different reader with a low solid limit
                ArcSnp solid_reader(10, 3, 10, 500);
                Content result = solid_reader.GetContent(path);
                if (result.empty() ||
                    result.back().info.state_ != ArcSnpState::kSolidFileSizeGreaterThanMax) {
                    Logger::WriteMessage(
                        "FAILURE: Solid archive test failed in parallel.");
                    all_threads_passed = false;
                }
            }
            catch (const std::exception& e) {
                Logger::WriteMessage(
                    ("FAILURE: Exception in solid test: " + std::string(e.what()))
                    .c_str());
                all_threads_passed = false;
            }
            };

        // Let's redefine future4 to be more robust
        auto future4_solid = std::async(std::launch::async, task_solid);

        // --- Wait for all tasks to complete ---
        future1.get();
        future2.get();
        future3.get();
        future4_solid.get();

        // The real test:
        // If all_threads_passed is true, it means no thread
        // failed its assertions *and* we didn't crash from data races,
        // which would have happened with the old implementation.
        Assert::IsTrue(all_threads_passed.load(),
            L"One or more parallel tasks failed. See output log.");
    }
    };
}  // namespace ConcurrencyTest