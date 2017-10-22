/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with class used to read files from PAK archive.
 */

#include "PCH.hpp"
#include "PackerReader.hpp"

namespace NFE {
namespace Common {

PackerReader::PackerReader(): mFileVersion(0) {}
PackerReader::PackerReader(const StringView filePath): mFileVersion(0)
{
    Init(filePath);
}

PackerResult PackerReader::Init(const StringView filePath)
{
    mFilePath = filePath;
    File file(mFilePath, AccessMode::Read);

    if (!(file.IsOpened()))
        return PackerResult::FileNotFound;

    if (file.GetSize() <= 0)
        return PackerResult::FileNotFound;

    if (!file.Read(&mFileVersion, sizeof(uint32)))
        return PackerResult::ReadFailed;

    size_t fileSize = 0;
    if (!file.Read(&fileSize, sizeof(size_t)))
        return PackerResult::ReadFailed;

    PackerElement tempElement;
    for (size_t i = 0; i < fileSize; ++i)
    {
        if (!file.Read(reinterpret_cast<void*>(&tempElement.mFilePos), sizeof(size_t)))
            return PackerResult::ReadFailed;

        if (!file.Read(reinterpret_cast<void*>(&tempElement.mHash), 4 * sizeof(uint32)))
            return PackerResult::ReadFailed;

        if (!file.Read(reinterpret_cast<void*>(&tempElement.mFileSize), sizeof(size_t)))
            return PackerResult::ReadFailed;

        mFileList.push_back(tempElement);
    }

    return PackerResult::OK;
}

PackerResult PackerReader::GetFile(const StringView vfsFilePath, Buffer& outputBuffer)
{
    if (mFileList.empty())
        return PackerResult::Uninitialized;

    MD5Hash searchHash;
    searchHash.Calculate(vfsFilePath);

    for (size_t i = 0; i < mFileList.size(); ++i)
    {
        if (searchHash == mFileList[i].mHash)
        {
            outputBuffer.Create(mFileList[i].mFileSize);

            File file(mFilePath, AccessMode::Read);

            size_t filePos = mFileList[i].mFilePos;
            if (!file.Seek(static_cast<int64>(filePos), SeekMode::Begin))
                return PackerResult::ReadFailed;

            if (!file.Read(outputBuffer.GetData(), mFileList[i].mFileSize))
                return PackerResult::ReadFailed;

            return PackerResult::OK;
        }
    }

    return PackerResult::FileNotFound;
}

void PackerReader::PrintFilesToStdout() const
{
    for (const auto& it : mFileList)
        std::cout << "Element " << it.mHash << ", size " << it.mFileSize << std::endl;
}

size_t PackerReader::GetFileCount() const
{
    return mFileList.size();
}

uint32 PackerReader::GetFileVersion() const
{
    return mFileVersion;
}

} // namespace Common
} // namespace NFE
