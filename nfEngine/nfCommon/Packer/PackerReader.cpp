/**
 * @file   PackerReader.cpp
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with class used to read files from PAK archive.
 */

#include "../stdafx.hpp"
#include "PackerReader.hpp"

namespace NFE {
namespace Common {

PackReader::PackReader(): mFileVersion(0) {}
PackReader::PackReader(const std::string& filePath): mFileVersion(0)
{
    Init(filePath);
}

PackResult PackReader::Init(const std::string& filePath)
{
    mFilePath = filePath;
    FILEPtr pFile(fopen(mFilePath.c_str(), "rb"), FILEPtrDestroy);

    if (!(pFile.get()))
        return PackResult::FileNotFound;

    if (!fread(&mFileVersion, sizeof(uint32), 1, pFile.get()))
        return PackResult::ReadFailed;

    size_t mFileSize = 0;
    if (!fread(&mFileSize, sizeof(size_t), 1, pFile.get()))
        return PackResult::ReadFailed;

    PackerElement tempElement;
    for (size_t i = 0; i < mFileSize; ++i)
    {
        if (!fread(reinterpret_cast<void*>(&tempElement.mHash), sizeof(uint32), 4, pFile.get()))
            return PackResult::ReadFailed;

        if (!fread(reinterpret_cast<void*>(&tempElement.FilePos), sizeof(size_t), 1, pFile.get()))
            return PackResult::ReadFailed;

        if (!fread(reinterpret_cast<void*>(&tempElement.FileSize), sizeof(size_t), 1, pFile.get()))
            return PackResult::ReadFailed;

        mFileList.push_back(tempElement);
    }

    return PackResult::OK;
}

PackResult PackReader::GetFile(const std::string& vfsFilePath, Buffer& outputBuffer)
{
    if (mFileList.empty())
        return PackResult::Uninitialized;

    MD5Hash searchHash;
    searchHash.Calculate(vfsFilePath);

    for (size_t i = 0; i < mFileList.size(); ++i)
    {
        if (searchHash == mFileList[i].mHash)
        {
            outputBuffer.Create(mFileList[i].FileSize);

            FILEPtr pFile(fopen(mFilePath.c_str(), "rb"), FILEPtrDestroy);

            size_t filePos = mFileList[i].FilePos;
            if (fseek(pFile.get(), static_cast<long>(filePos), SEEK_SET))
                return PackResult::ReadFailed;

            if (!fread(outputBuffer.GetData(), sizeof(unsigned char), mFileList[i].FileSize, pFile.get()))
                return PackResult::ReadFailed;

            return PackResult::OK;
        }
    }

    return PackResult::FileNotFound;
}

void PackReader::PrintFilesToStdout() const
{
    for (const auto& it : mFileList)
        std::cout << "Element " << it.mHash << ", size " << it.FileSize << std::endl;
}

size_t PackReader::GetFileCount() const
{
    return mFileList.size();
}

uint32 PackReader::GetFileVersion() const
{
    return mFileVersion;
}

} // namespace Common
} // namespace NFE
