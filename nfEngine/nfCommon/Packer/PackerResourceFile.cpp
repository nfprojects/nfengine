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
    FilePtr pFile(new File(filePath, AccessMode::Read));
    if (!(pFile.get()->IsOpened()))
        return PackerResult::FileNotFound;

    mFilePath = filePath;

    mFileSize = pFile.get()->GetSize();
    mHash.Calculate(vfsFilePath);

    return PackerResult::OK;
}

PackerResult PackerResourceFile::Save(File* file)
{
    // It was "rb" in here
    FilePtr appendedFile(new File(mFilePath, AccessMode::Read));
    if (!(appendedFile.get()->IsOpened()))
        return PackerResult::FileNotFound;

    size_t readCount;
    unsigned char buffer[PACKER_DEF_BUFFER_SIZE];
    do
    {
        readCount = appendedFile.get()->Read(buffer, PACKER_DEF_BUFFER_SIZE);
        if (file->Write(buffer, readCount) != readCount)
            return PackerResult::WriteFailed;
    }
    while (readCount == PACKER_DEF_BUFFER_SIZE);

    return PackerResult::OK;
}

} // namespace Common
} // namespace NFE
