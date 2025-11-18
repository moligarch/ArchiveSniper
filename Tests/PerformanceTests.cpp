#include "pch.h"

namespace PerformanceTest {
    TEST_CLASS(PressureTests) {
public:
    //TEST_METHOD(SingleLargeFile) {
    //    std::string path =
    //        "..\\..\\..\\..\\Tests\\performance\\load\\large.rar";
    //    // Use new 4-arg constructor
    //    ArcSnp reader(200, 50, 500, 1000);  // 1000MB memory limit
    //    Content result;
    //    try {
    //        result = reader.GetContent(path);
    //        Assert::IsFalse(result.empty());
    //        // Use new member name: state_
    //        Assert::IsTrue(
    //            result.back().info.state_ == ArcSnpState::kStackFilled,
    //            std::wstring(std::format(L"Result ends with {} state",
    //                int(result.back().info.state_)))
    //            .c_str());
    //    }
    //    catch (const std::exception& e) {
    //        Logger::WriteMessage(e.what());
    //        Assert::Fail(L"Test threw an exception.");
    //    }
    //};
    TEST_METHOD(MultipleFiles) {
        std::string base_path =
            "..\\..\\..\\..\\Tests\\performance\\load\\# (";
        Content result;

        for (int i = 1; i <= 30; i++) {
            std::string path = base_path + std::to_string(i) + ").zip";
            // Use new 4-arg constructor
            ArcSnp reader(10, 10, 10, 500);
            Content output = reader.GetContent(path);
            result.insert(result.end(), output.begin(), output.end());
        }
        Assert::IsTrue(
            result.size() == 30,
            std::wstring(std::format(L"Result contains {} items", result.size()))
            .c_str());
    };
    };
    TEST_CLASS(RecursionTests) {
public:
    // Helper to create a reader with consistent default limits for tests
    ArcSnp CreateReader(size_t file = 5, size_t solid = 5, size_t buffer = 10,
        size_t memory = 500) {
        return ArcSnp(file, solid, buffer, memory);
    }

    // Helper for recursive find assertions
    void FindNestedFile(const std::string& path, int depth) {
        auto reader = CreateReader();
        Content result;
        try {
            result = reader.GetContent(path, depth);
            auto iter =
                std::find_if(result.begin(), result.end(),
                    [&path, depth](const Decompressed& item) {
                        // Build the expected path dynamically
                        std::string expected_path = path;
                        for (int d = depth - 1; d >= 1; --d) {
                            expected_path +=
                                "->depth" + std::to_string(d) + ".zip";
                        }
                        expected_path += "->test.txt";
                        // Use new member name: base_path_
                        return item.info.base_path_ == expected_path;
                    });

            Assert::IsTrue(iter != result.end(), L"Nested test file not found.");
            // Use new member name: buffer_
            std::string buffer(iter->buffer_.begin(), iter->buffer_.end());
            Assert::IsTrue(buffer == "Find This For Test");

        }
        catch (const std::exception& e) {
            Logger::WriteMessage(e.what());
            Assert::Fail(L"Test threw an exception.");
        }
    }

    TEST_METHOD(Depth3) {
        FindNestedFile(
            "..\\..\\..\\..\\Tests\\performance\\recursion\\depth3.zip",
            3);
    }
    TEST_METHOD(Depth4) {
        FindNestedFile(
            "..\\..\\..\\..\\Tests\\performance\\recursion\\depth4.zip",
            4);
    }
    TEST_METHOD(Depth5) {
        FindNestedFile(
            "..\\..\\..\\..\\Tests\\performance\\recursion\\depth5.zip",
            5);
    }
    TEST_METHOD(Depth6) {
        FindNestedFile(
            "..\\..\\..\\..\\Tests\\performance\\recursion\\depth6.zip",
            6);
    }
    TEST_METHOD(Depth7) {
        FindNestedFile(
            "..\\..\\..\\..\\Tests\\performance\\recursion\\depth7.zip",
            7);
    }
    TEST_METHOD(Depth8) {
        FindNestedFile(
            "..\\..\\..\\..\\Tests\\performance\\recursion\\depth8.zip",
            8);
    }
    TEST_METHOD(Depth9) {
        FindNestedFile(
            "..\\..\\..\\..\\Tests\\performance\\recursion\\depth9.zip",
            9);
    }
    TEST_METHOD(DepthFinal) {
        std::string path =
            "..\\..\\..\\..\\Tests\\performance\\recursion\\depth1000."
            "zip";
        auto reader = CreateReader(5, 5, 10, 500);
        Content result;
        try {
            result = reader.GetContent(path, 1000);
            Assert::IsFalse(result.empty(), L"Result was empty.");

            // Just check the first item found
            // Use new member name: buffer_
            std::string buffer(result[0].buffer_.begin(), result[0].buffer_.end());
            Assert::IsTrue(buffer.find("Find This For Test") != std::string::npos);

        }
        catch (const std::exception& e) {
            Logger::WriteMessage(e.what());
            Assert::Fail(L"Test threw an exception.");
        }
    }
    };
    TEST_CLASS(Vulnerability) {
public:
    // Helper to create a reader with consistent default limits for tests
    ArcSnp CreateReader(size_t file = 10, size_t solid = 10, size_t buffer = 10,
        size_t memory = 500) {
        return ArcSnp(file, solid, buffer, memory);
    }

    TEST_METHOD(CVE2021_40444_Infected) {
        std::string infected_doc =
            "..\\..\\..\\..\\Tests\\performance\\vulnerability\\"
            "CVE2021_40444\\CVE2021_40444.rar";
        auto reader = CreateReader(5, 5, 10, 500);
        Content infected_result;
        try {
            infected_result = reader.GetContent(infected_doc, 3);
            auto iter = std::find_if(
                infected_result.begin(), infected_result.end(),
                [&infected_doc](const Decompressed& item) {
                    // Use new member name: base_path_
                    return item.info.base_path_ ==
                        infected_doc + "->infected.docx->word\\_rels\\document.xml.rels";
                });
            Assert::IsTrue(iter != infected_result.end(), L"rels file not found.");

            // Use new member name: buffer_
            std::string buffer(iter->buffer_.begin(), iter->buffer_.end());
            bool is_infected = buffer.find("mhtml") != std::string::npos;
            Assert::IsTrue(is_infected);
        }
        catch (const std::exception& e) {
            Logger::WriteMessage(e.what());
            Assert::Fail(L"Test threw an exception.");
        }
    };
    TEST_METHOD(CVE2021_40444_Clean) {
        std::string clean_doc =
            "..\\..\\..\\..\\Tests\\performance\\vulnerability\\"
            "CVE2021_40444\\CVE2021_40444.rar";
        auto reader = CreateReader(5, 5, 10, 500);
        Content clean_result;
        try {
            clean_result = reader.GetContent(clean_doc, 3);
            auto iter = std::find_if(clean_result.begin(), clean_result.end(),
                [&clean_doc](const Decompressed& item) {
                    // Use new member name: base_path_
                    return item.info.base_path_ ==
                        clean_doc +
                        "->clean.docx->word\\_rels\\document.xml.rels";
                });
            Assert::IsTrue(iter != clean_result.end(), L"rels file not found.");

            // Use new member name: buffer_
            std::string buffer(iter->buffer_.begin(), iter->buffer_.end());
            bool is_infected = buffer.find("mhtml") != std::string::npos;
            Assert::IsFalse(is_infected);
        }
        catch (const std::exception& e) {
            Logger::WriteMessage(e.what());
            Assert::Fail(L"Test threw an exception.");
        }
    };
    TEST_METHOD(ZipBomb_with_0) {
        std::string zip_bomb =
            "..\\..\\..\\..\\Tests\\performance\\vulnerability\\"
            "ZipBomb\\ZipBomb0.zip";
        auto reader = CreateReader(20, 20, 500, 1000);
        Content result;
        try {
            result = reader.GetContent(zip_bomb, 5);
            Assert::IsFalse(result.empty());
            // Use new member name: state_
            Assert::IsTrue(result[0].info.state_ == ArcSnpState::kUnsafe);
        }
        catch (const std::exception& e) {
            Logger::WriteMessage(e.what());
            Assert::Fail(L"Test threw an exception.");
        }
    };
    TEST_METHOD(ZipBomb_with_1) {
        std::string zip_bomb =
            "..\\..\\..\\..\\Tests\\performance\\vulnerability\\"
            "ZipBomb\\ZipBomb1.zip";
        auto reader = CreateReader(20, 20, 500, 1000);
        Content result;
        try {
            result = reader.GetContent(zip_bomb, 5);
            Assert::IsFalse(result.empty());
            // Use new member name: state_
            Assert::IsTrue(
                result.back().info.state_ == ArcSnpState::kUnsafe,
                std::wstring(std::format(L"Result ends with state {}",
                    int(result.back().info.state_)))
                .c_str());
        }
        catch (const std::exception& e) {
            Logger::WriteMessage(e.what());
            Assert::Fail(L"Test threw an exception.");
        }
    };
    TEST_METHOD(ZipBomb_with_0110) {
        std::string zip_bomb =
            "..\\..\\..\\..\\Tests\\performance\\vulnerability\\"
            "ZipBomb\\ZipBomb0110.zip";
        auto reader = CreateReader(20, 20, 500, 1000);
        Content result;
        try {
            result = reader.GetContent(zip_bomb, 5);
            Assert::IsFalse(result.empty());
            // Use new member name: state_
            Assert::IsTrue(result[0].info.state_ == ArcSnpState::kUnsafe);
        }
        catch (const std::exception& e) {
            Logger::WriteMessage(e.what());
            Assert::Fail(L"Test threw an exception.");
        }
    };
    };
}  // namespace PerformanceTest