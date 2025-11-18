#include "pch.h"

// This operator is used by the GetContentList tests
// It now correctly uses the new ArcInfo::operator==
bool operator==(const ContentList& lhs, const ContentList& rhs) {
    return lhs.size() == rhs.size() &&
        std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

// Global initialization for all tests in this module
TEST_MODULE_INITIALIZE(ModuleInitialize) {
    try {
        // We must initialize the 7zip.dll library *once* before any
        // ArcSnp objects can be created. We'll assume 7zip.dll is
        // in the same directory as the test executable.
        Shared7z::Init("7zip.dll");
        Logger::WriteMessage("Shared7z (7zip.dll) initialized successfully.");
    }
    catch (const std::exception& e) {
        std::string msg = "CRITICAL FAILURE: Failed to initialize Shared7z: ";
        msg += e.what();
        Logger::WriteMessage(msg.c_str());
        // Fail the entire test run if the DLL can't be loaded.
        Assert::Fail(L"ModuleInitialize failed to load 7zip.dll.");
    }
}