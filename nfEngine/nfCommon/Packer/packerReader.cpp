/**
    NFEngine project

    \file   packerReader.cpp
    \author LKostyra (costyrra.xl@gmail.com)
    \brief  Module with class used to read files from PAK archive.
*/

#include "stdafx.h"
#include "packerReader.hpp"

namespace NFE {
namespace Common {

PackReader::PackReader(): mFileVersion(0) {}
PackReader::PackReader(const std::string& FilePath): mFileVersion(0)
{
    Init(FilePath);
}

PACK_RESULT PackReader::Init(const std::string& FilePath)
{
    mFilePath = FilePath;
    FILEPtr pFile(fopen(mFilePath.c_str(), "rb"), FILEPtrDestroy);

    if (!(pFile.get()))
        return PACK_RESULT::FILE_NOT_FOUND;

    if (!fread(&mFileVersion, sizeof(uint32), 1, pFile.get()))
        return PACK_RESULT::READ_FAILED;

    size_t mFileSize = 0;
    if (!fread(&mFileSize, sizeof(size_t), 1, pFile.get()))
        return PACK_RESULT::READ_FAILED;

    packElement tempElement;
    for (size_t i = 0; i < mFileSize; ++i)
    {
        uint32 hash, fullHash[4];

        if (!fread(reinterpret_cast<void*>(&tempElement.mHash), sizeof(uint32), 4, pFile.get()))
            return PACK_RESULT::READ_FAILED;

        if (!fread(reinterpret_cast<void*>(&tempElement.FilePos), sizeof(long), 1, pFile.get()))
            return PACK_RESULT::READ_FAILED;

        if (!fread(reinterpret_cast<void*>(&tempElement.FileSize), sizeof(size_t), 1, pFile.get()))
            return PACK_RESULT::READ_FAILED;

        mFileList.push_back(tempElement);
    }

    return PACK_RESULT::OK;
}

PACK_RESULT PackReader::GetFile(const std::string& VFSFilePath, Buffer& outbuf)
{
    if (mFileList.empty())
        return PACK_RESULT::UNINITIALIZED;

    MD5Hash searchHash;
    searchHash.Calculate(VFSFilePath);

    for (size_t i = 0; i < mFileList.size(); ++i)
    {
        if (searchHash == mFileList[i].mHash)
        {
            outbuf.Create(mFileList[i].FileSize);

            FILEPtr pFile(fopen(mFilePath.c_str(), "rb"), FILEPtrDestroy);

            long l_FilePos = mFileList[i].FilePos;
            if (fseek(pFile.get(), l_FilePos, SEEK_SET))
                return PACK_RESULT::READ_FAILED;

            if (!fread(outbuf.GetData(), sizeof(unsigned char), mFileList[i].FileSize, pFile.get()))
                return PACK_RESULT::READ_FAILED;

            return PACK_RESULT::OK;
        }
    }

    return PACK_RESULT::FILE_NOT_FOUND;
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
