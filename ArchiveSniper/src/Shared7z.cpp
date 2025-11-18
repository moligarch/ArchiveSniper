#include "ArcSnp/Shared7z.h"

#include <stdexcept>

// Initialize static members
std::unique_ptr<Shared7z> Shared7z::instance_{ nullptr };
std::once_flag Shared7z::init_flag_;

Shared7z::Shared7z() : lib_{ nullptr } {}

void Shared7z::Initialize(const fs::path& library_path) {
    // The bit7z::Bit7zLibrary constructor will throw a BitException
    // if it fails to load the DLL from the provided path.
    lib_ = std::make_shared<bit7z::Bit7zLibrary>(library_path.string());
}

void Shared7z::Init(const fs::path& library_path) {
    std::call_once(init_flag_, [&]() {
        instance_.reset(new Shared7z());
        try {
            instance_->Initialize(library_path);
        }
        catch (...) {
            // If Initialize fails (e.g., DLL not found), reset the instance
            // so GetInstance() will correctly throw an error.
            instance_.reset();
            throw;  // Re-throw the original exception
        }
        });
}

Shared7z& Shared7z::GetInstance() {
    if (!instance_) {
        // This will be true if Init() was never called, or if Init() failed.
        throw std::runtime_error(
            "Shared7z::Init must be called successfully before GetInstance.");
    }
    return *instance_;
}

std::shared_ptr<bit7z::Bit7zLibrary> Shared7z::GetLibrary() {
    // This is now the single public access point.
    // It retrieves the singleton instance and returns the library pointer.
    return GetInstance().lib_;
}