#include "ArchiveSniper.h"
#include "ArcSnpImpl.h"
#include <fstream>

ArcSnp::ArcSnpImpl::ArcSnpImpl(const std::string& path, bool allowRecursive, DWORD depthLimit) :
    mPath(path), mRecursion(allowRecursive), mDepthLimit(depthLimit)
{
    mLib = sLib::Instance().LibAccess();
}
ArcSnp::ArcSnpImpl::ArcSnpImpl(const std::string& path, bool allowRecursive) :
    mPath(path), mRecursion(allowRecursive)
{
    mLib = sLib::Instance().LibAccess();
}
ArcSnp::ArcSnpImpl::~ArcSnpImpl()
{
}

META ArcSnp::ArcSnpImpl::GetMetadata(const std::string& filePath) {

    try { // bit7z classes can throw BitException objects

        bit7z::BitArchiveReader arc{ *mLib, filePath, bit7z::BitFormat::Auto};

        META result{ arc.itemsCount(),arc.foldersCount(), arc.filesCount(), arc.size(), arc.packSize(), arc.format().value(), std::error_code{} , "" };

        return result;
    }
    catch (const bit7z::BitException& ex)
    {
        META failed{ NULL,NULL,NULL,NULL,NULL,NULL,ex.code(),ex.what()};
        return failed;
    }
}
content_t ArcSnp::ArcSnpImpl::GetContent(std::string& path)
{
    if (mDepth++ >= mDepthLimit)
    {
        mRecursion = false;
    }
    content_t result;
    if (!mRecursion)
    {
        bit7z::BitArchiveReader arc{ *mLib , path, bit7z::BitFormat::Auto};
        auto arc_items = arc.items();
        for (const auto& item : arc_items) {
            bit7z::buffer_t buffer{};
            arc.extractTo(buffer, item.index());
            result.emplace_back(DCOMP{
                path + "->" + item.path(),
                kBaseArchiveLevel,
                buffer });
        }
    }
    else
    {
        bit7z::BitArchiveReader arc{ *mLib, path, bit7z::BitFormat::Auto };
        auto arc_items = arc.items();
        for (const auto& item : arc_items) {
            bit7z::buffer_t tmpBuffer{};
            arc.extractTo(tmpBuffer, item.index());
            if (CheckBuffer(item))
            {   
                auto output = ResolveBuffer(path + "->" + item.path(), tmpBuffer);
                result.insert(result.end(), output.begin(), output.end());
            }
            else
            {
                result.emplace_back(DCOMP{
                    path + "->" + item.path(),
                    mDepth,
                    tmpBuffer });
            }
        }
    }
    mDepth--;
    return result;
}
content_t ArcSnp::ArcSnpImpl::ResolveBuffer(const std::string& basePath, bit7z::buffer_t& buffer)
{
    if (mDepth++ >= mDepthLimit)
    {
        mRecursion = false;
    }

    content_t result;
    if (!mRecursion)
    {
        bit7z::BitArchiveReader arc{ *mLib, buffer, bit7z::BitFormat::Auto };
        auto arc_items = arc.items();
        for (const auto& item : arc_items) {
            bit7z::buffer_t tmpBuffer{};
            arc.extractTo(tmpBuffer, item.index());
            result.emplace_back(DCOMP{
                basePath + "->" + item.path(),
                mDepth,
                tmpBuffer });
        }
    }
    else
    {
        try
        {
            bit7z::BitArchiveReader arc{ *mLib, buffer, bit7z::BitFormat::Auto };
            auto arc_items = arc.items();
            for (const auto& item : arc_items) {
                bit7z::buffer_t tmpBuffer{};
                arc.extractTo(tmpBuffer, item.index());
                if (CheckBuffer(item))
                {
                    auto output = ResolveBuffer(basePath + "->" + item.path(), tmpBuffer);
                    result.insert(result.end(), output.begin(), output.end());
                }
                else
                {
                    result.emplace_back(DCOMP{
                        basePath + "->" + item.path(),
                        mDepth,
                        tmpBuffer });
                }
            }
        }
        catch (const bit7z::BitException&)
        {
            result.emplace_back(DCOMP{
                basePath,
                mDepth - 1,
                buffer
                });
        }
    }
    mDepth--;
    return result;
}
bool ArcSnp::ArcSnpImpl::CheckBuffer(const bit7z::BitArchiveItemInfo& itemInfo)
{
    return itemInfo.size() / 1024 <= kMaxFileBufferSize;

}

ArcSnp::ArcSnp(const std::string& path, bool allowRecursive, DWORD depthLimit) :
    mpImpl(new ArcSnpImpl(path, allowRecursive, depthLimit))
{
}
ArcSnp::ArcSnp(const std::string& path, bool allowRecursive) :
    mpImpl(new ArcSnpImpl(path, allowRecursive))
{
}
ArcSnp::~ArcSnp()
{
}

META ArcSnp::GetMetadata(const std::string& filePath)
{
    return mpImpl->GetMetadata(filePath);
}
content_t ArcSnp::GetContent(std::string& path)
{
    return mpImpl->GetContent(path);
}
