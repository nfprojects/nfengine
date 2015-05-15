/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with class used to read files from PAK archive.
 */

#include "../PCH.hpp"
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
    FILEPtr pFile(fopen(mFilePath.c_str(), "rb"), FILEPtrDestroy);

    if (!(pFile.get()))
        return PackerResult::FileNotFound;

    if (!fread(&mFileVersion, sizeof(uint32), 1, pFile.get()))
        return PackerResult::ReadFailed;

    size_t fileSize = 0;
    if (!fread(&fileSize, sizeof(size_t), 1, pFile.get()))
        return PackerResult::ReadFailed;

    PackerElement tempElement;
    for (size_t i = 0; i < fileSize; ++i)
    {
        if (!fread(reinterpret_cast<void*>(&tempElement.mFilePos), sizeof(size_t), 1, pFile.get()))
            return PackerResult::ReadFailed;

        if (!fread(reinterpret_cast<void*>(&tempElement.mHash), sizeof(uint32), 4, pFile.get()))
            return PackerResult::ReadFailed;

        if (!fread(reinterpret_cast<void*>(&tempElement.mFileSize), sizeof(size_t), 1, pFile.get()))
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

            FILEPtr pFile(fopen(mFilePath.c_str(), "rb"), FILEPtrDestroy);

            size_t filePos = mFileList[i].mFilePos;
            if (fseek(pFile.get(), static_cast<long>(filePos), SEEK_SET))
                return PackerResult::ReadFailed;

            if (!fread(outputBuffer.GetData(), sizeof(unsigned char), mFileList[i].mFileSize, pFile.get()))
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
