#include "pch.h"

namespace UnitTests {
    TEST_CLASS(GetMetadata) {
public:
    // Helper to create a reader with consistent default limits for tests
    ArcSnp CreateReader(size_t file = 10, size_t solid = 10, size_t buffer = 10,
        size_t memory = 500) {
        return ArcSnp(file, solid, buffer, memory);
    }

    TEST_METHOD(Doc) {
        const std::string path =
            "..\\..\\..\\..\\Tests\\unit\\test.doc";
        auto reader = CreateReader();
        auto result = reader.GetMetadata(path);
        if (std::holds_alternative<Meta>(result)) {
            auto meta = std::get<Meta>(result);
            Assert::IsTrue(meta.files_count_ > 5);
            Assert::AreEqual(meta.GetExtension().c_str(), "compound");
        }
        else {
            Assert::Fail(L"Expected metadata, got error state.");
        }
    }
    TEST_METHOD(Docx) {
        const std::string path =
            "..\\..\\..\\..\\Tests\\unit\\test.docx";
        auto reader = CreateReader();
        auto result = reader.GetMetadata(path);
        Assert::IsTrue(std::holds_alternative<Meta>(result));
        auto meta = std::get<Meta>(result);
        Assert::IsTrue(meta.files_count_ > 5);
        Assert::AreEqual(meta.GetExtension().c_str(), "zip");
    }
    TEST_METHOD(Msi) {
        const std::string path =
            "..\\..\\..\\..\\Tests\\unit\\test.msi";
        auto reader = CreateReader();
        auto result = reader.GetMetadata(path);
        Assert::IsTrue(std::holds_alternative<Meta>(result));
        auto meta = std::get<Meta>(result);
        Assert::IsTrue(meta.files_count_ > 5);
        Assert::AreEqual(meta.GetExtension().c_str(), "compound");
    }
    TEST_METHOD(Rar) {
        const std::string path =
            "..\\..\\..\\..\\Tests\\unit\\test.rar";
        auto reader = CreateReader();
        auto result = reader.GetMetadata(path);
        Assert::IsTrue(std::holds_alternative<Meta>(result));
        auto meta = std::get<Meta>(result);
        Assert::IsTrue(meta.files_count_ > 0);
        Assert::AreEqual(meta.GetExtension().c_str(), "rar");
    }
    TEST_METHOD(SevenZ) {
        const std::string path =
            "..\\..\\..\\..\\Tests\\unit\\test.7z";
        auto reader = CreateReader();
        auto result = reader.GetMetadata(path);
        Assert::IsTrue(std::holds_alternative<Meta>(result));
        auto meta = std::get<Meta>(result);
        Assert::IsTrue(meta.files_count_ > 0);
        Assert::AreEqual(meta.GetExtension().c_str(), "7z");
    }
    TEST_METHOD(Sfx) {
        const std::string path =
            "..\\..\\..\\..\\Tests\\unit\\test.sfx";
        auto reader = CreateReader();
        auto result = reader.GetMetadata(path);
        Assert::IsTrue(std::holds_alternative<Meta>(result));
        auto meta = std::get<Meta>(result);
        Assert::IsTrue(meta.files_count_ > 4);
        Assert::AreEqual(meta.GetExtension().c_str(), "pe");
    }
    };

    TEST_CLASS(GetContent) {
public:
    // Helper to create a reader with consistent default limits for tests
    ArcSnp CreateReader(size_t file = 10, size_t solid = 10, size_t buffer = 10,
        size_t memory = 500) {
        return ArcSnp(file, solid, buffer, memory);
    }

    TEST_METHOD(Normal) {
        std::string path = "..\\..\\..\\..\\Tests\\unit\\test.docx";
        auto reader = CreateReader();
        try {
            auto result = reader.GetContent(path);
            auto iter = std::find_if(result.begin(), result.end(),
                [&path](const Decompressed& item) {
                    // Use new member name: base_path_
                    return item.info.base_path_ ==
                        path + "->test.txt";
                });
            Assert::IsTrue(iter != result.end(), L"Test file 'test.txt' not found.");

            // Use new member name: buffer_
            // Use efficient string constructor
            std::string buffer(iter->buffer_.begin(), iter->buffer_.end());
            Assert::IsTrue(buffer == "Find This For Test");
        }
        catch (const std::exception& e) {
            Logger::WriteMessage(e.what());
            Assert::Fail(L"Test threw an exception.");
        }
    }
    TEST_METHOD(SolidArchive) {
        std::string path = "..\\..\\..\\..\\Tests\\unit\\solid.rar";
        // Set solid limit to 3, memory to 500
        auto reader = CreateReader(10, 3, 10, 500);
        Content result;
        try {
            result = reader.GetContent(path);
            Assert::IsFalse(result.empty());
            // Use new member names: base_path_, buffer_, state_
            Assert::IsTrue(result.back().info.base_path_ == path);
            Assert::IsTrue(result.back().buffer_.empty());
            auto state = result.back().info.state_;
            Assert::IsTrue(
                state == ArcSnpState::kSolidFileSizeGreaterThanMax,
                std::wstring(std::format(L"Result ends with {} state", int(state)))
                .c_str());
        }
        catch (const std::exception& e) {
            Logger::WriteMessage(e.what());
            Assert::Fail(L"Test threw an exception.");
        }
    }
    TEST_METHOD(Nested) {
        std::string path = "..\\..\\..\\..\\Tests\\unit\\depth2.zip";
        // Set buffer limit to 5, memory to 500
        auto reader = CreateReader(10, 10, 5, 500);
        Content result;
        try {
            result = reader.GetContent(path, 2);
            auto iter = std::find_if(result.begin(), result.end(),
                [&path](const Decompressed& item) {
                    // Use new member name: base_path_
                    return item.info.base_path_ ==
                        path + "->depth1.zip->test.txt";
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
    };

    TEST_CLASS(GetContentList) {
public:
    // Helper to create a reader with consistent default limits for tests
    ArcSnp CreateReader(size_t file = 10, size_t solid = 10, size_t buffer = 10,
        size_t memory = 500) {
        return ArcSnp(file, solid, buffer, memory);
    }

    TEST_METHOD(Normal) {
        std::string path = "..\\..\\..\\..\\Tests\\unit\\testList.zip";
        auto reader = CreateReader();
        try {
            auto result = reader.GetContentList(path);
            // Use new member names: base_path_, depth_
            ContentList expected = {
                {.base_path_ = path + "->test.txt", .depth_ = 1},
                {.base_path_ = path + "->test - Copy.txt", .depth_ = 1},
                {.base_path_ = path + "->test - Copy (2).txt", .depth_ = 1},
                {.base_path_ = path + "->test - Copy (3).txt", .depth_ = 1},
            };
            Assert::IsTrue(result == expected);
        }
        catch (const std::exception& e) {
            Logger::WriteMessage(e.what());
            Assert::Fail(L"Test threw an exception.");
        }
    };
    TEST_METHOD(Nested) {
        std::string path =
            "..\\..\\..\\..\\Tests\\unit\\testListNested.rar";
        auto reader = CreateReader();
        try {
            auto result = reader.GetContentList(path, 2);
            // Use new member names: base_path_, depth_
            ContentList expected = {
                {.base_path_ = path + "->testList.zip->test.txt", .depth_ = 2},
                {.base_path_ = path + "->testList.zip->test - Copy.txt",
                 .depth_ = 2},
                {.base_path_ = path + "->testList.zip->test - Copy (2).txt",
                 .depth_ = 2},
                {.base_path_ = path + "->testList.zip->test - Copy (3).txt",
                 .depth_ = 2},
                {.base_path_ = path + "->testList1.zip->test.txt", .depth_ = 2},
                {.base_path_ = path + "->testList1.zip->test - Copy.txt",
                 .depth_ = 2},
                {.base_path_ = path + "->testList1.zip->test - Copy (2).txt",
                 .depth_ = 2},
                {.base_path_ = path + "->testList1.zip->test - Copy (3).txt",
                 .depth_ = 2},
                {.base_path_ = path + "->testList2.zip->test.txt", .depth_ = 2},
                {.base_path_ = path + "->testList2.zip->test - Copy.txt",
                 .depth_ = 2},
                {.base_path_ = path + "->testList2.zip->test - Copy (2).txt",
                 .depth_ = 2},
                {.base_path_ = path + "->testList2.zip->test - Copy (3).txt",
                 .depth_ = 2},
            };
            Assert::IsTrue(result == expected);
        }
        catch (const std::exception& e) {
            Logger::WriteMessage(e.what());
            Assert::Fail(L"Test threw an exception.");
        }
    };
    };

    TEST_CLASS(Write) {
public:
    // TODO
    };
}  // namespace UnitTests