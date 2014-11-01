/**
 * @file   PackerResourceFile.cpp
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with class representing VFS file from file on disk
 */

#include "../stdafx.hpp"

#include "PackerResourceFile.hpp"

namespace NFE {
namespace Common {

PackerResult PackerResourceFile::Init(const std::string& filePath, const std::string& vfsFilePath)
{
    FILEPtr pFile(fopen(filePath.c_str(), "r"), FILEPtrDestroy);
    if (!(pFile.get()))
        return PackerResult::FileNotFound;

    mFilePath = filePath;

    fseek(pFile.get(), 0, SEEK_END);
    mFileSize = ftell(pFile.get());
    mHash.Calculate(vfsFilePath);

    return PackerResult::OK;
}

PackerResult PackerResourceFile::Save(FILE* file)
{
    FILEPtr appendedFile(fopen(mFilePath.c_str(), "rb"), FILEPtrDestroy);
    if (appendedFile.get() == NULL)
        return PackerResult::FileNotFound;

    size_t readCount;
    unsigned char buffer[PACKER_DEF_BUFFER_SIZE];
    do
    {
        readCount = fread(buffer, sizeof(unsigned char), PACKER_DEF_BUFFER_SIZE, appendedFile.get());
        if (fwrite(buffer, sizeof(unsigned char), readCount, file) != readCount)
            return PackerResult::WriteFailed;
    }
    while (readCount == PACKER_DEF_BUFFER_SIZE);

    return PackerResult::OK;
}

} // namespace Common
} // namespace NFE
