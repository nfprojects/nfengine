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

    uint32 numFiles = 0;
    if (!file.Read(&numFiles, sizeof(uint32)))
        return PackerResult::ReadFailed;

    PackerElement tempElement;
    for (uint32 i = 0; i < numFiles; ++i)
    {
        if (!file.Read(&tempElement.mFilePos, sizeof(uint64)))
            return PackerResult::ReadFailed;

        if (!file.Read(&tempElement.mHash, 4 * sizeof(uint32)))
            return PackerResult::ReadFailed;

        if (!file.Read(&tempElement.mFileSize, sizeof(uint32)))
            return PackerResult::ReadFailed;

        mFileList.PushBack(tempElement);
    }

    return PackerResult::OK;
}

PackerResult PackerReader::GetFile(const StringView vfsFilePath, Buffer& outputBuffer)
{
    if (mFileList.Empty())
        return PackerResult::Uninitialized;

    MD5Hash searchHash;
    searchHash.Calculate(vfsFilePath);

    for (uint32 i = 0; i < mFileList.Size(); ++i)
    {
        if (searchHash == mFileList[i].mHash)
        {
            outputBuffer.Create(mFileList[i].mFileSize);

            File file(mFilePath, AccessMode::Read);

            uint64 filePos = mFileList[i].mFilePos;
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

uint32 PackerReader::GetFileCount() const
{
    return mFileList.Size();
}

uint32 PackerReader::GetFileVersion() const
{
    return mFileVersion;
}

} // namespace Common
} // namespace NFE
