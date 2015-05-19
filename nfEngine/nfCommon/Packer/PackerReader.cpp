/**
 * @file   PackerReader.cpp
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with class used to read files from PAK archive.
 */

#include "../stdafx.hpp"
#include "PackerReader.hpp"

namespace NFE {
namespace Common {

PackerReader::PackerReader(): mFileVersion(0) {}
PackerReader::PackerReader(const std::string& filePath): mFileVersion(0)
{
    Init(filePath);
}

PackerResult PackerReader::Init(const std::string& filePath)
{
    mFilePath = filePath;
    // It was "rb" in here
    FilePtr pFile(new File(mFilePath, AccessMode::Read));

    // that is kind of weird
    if (!(pFile.get()->IsOpened()))
        return PackerResult::FileNotFound;

    if (pFile.get()->GetSize() <= 0)
        return PackerResult::FileNotFound;

    if (!pFile.get()->Read(&mFileVersion, sizeof(uint32)))
        return PackerResult::ReadFailed;

    size_t fileSize = 0;
    if (!pFile.get()->Read(&fileSize, sizeof(size_t)))
        return PackerResult::ReadFailed;

    PackerElement tempElement;
    for (size_t i = 0; i < fileSize; ++i)
    {
        if (!pFile.get()->Read(reinterpret_cast<void*>(&tempElement.mFilePos), sizeof(size_t)))
            return PackerResult::ReadFailed;

        if (!pFile.get()->Read(reinterpret_cast<void*>(&tempElement.mHash), 4 * sizeof(uint32)))
            return PackerResult::ReadFailed;

        if (!pFile.get()->Read(reinterpret_cast<void*>(&tempElement.mFileSize), sizeof(size_t)))
            return PackerResult::ReadFailed;

        mFileList.push_back(tempElement);
    }

    return PackerResult::OK;
}

PackerResult PackerReader::GetFile(const std::string& vfsFilePath, Buffer& outputBuffer)
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

            // It was "rb" in here
            FilePtr pFile(new File(mFilePath, AccessMode::Read));

            size_t filePos = mFileList[i].mFilePos;
            if (!pFile.get()->Seek(static_cast<int64>(filePos), SeekMode::Begin))
                return PackerResult::ReadFailed;

            if (!pFile.get()->Read(outputBuffer.GetData(), mFileList[i].mFileSize))
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
