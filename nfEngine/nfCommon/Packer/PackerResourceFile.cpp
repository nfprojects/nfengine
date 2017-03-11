/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with class representing VFS file from file on disk
 */

#include "PCH.hpp"

#include "PackerResourceFile.hpp"

namespace NFE {
namespace Common {

PackerResourceFile::~PackerResourceFile()
{
}

PackerResult PackerResourceFile::Init(const std::string& filePath, const std::string& vfsFilePath)
{
    File file(filePath, AccessMode::Read);
    if (!(file.IsOpened()))
        return PackerResult::FileNotFound;

    mFilePath = filePath;

    mFileSize = static_cast<size_t>(file.GetSize());
    mHash.Calculate(vfsFilePath);

    return PackerResult::OK;
}

PackerResult PackerResourceFile::Save(File& file)
{
    File appendedFile(mFilePath, AccessMode::Read);
    if (!(appendedFile.IsOpened()))
        return PackerResult::FileNotFound;

    size_t readCount;
    unsigned char buffer[PACKER_DEF_BUFFER_SIZE];
    do
    {
        readCount = appendedFile.Read(buffer, PACKER_DEF_BUFFER_SIZE);
        if (file.Write(buffer, readCount) != readCount)
            return PackerResult::WriteFailed;
    }
    while (readCount == PACKER_DEF_BUFFER_SIZE);

    return PackerResult::OK;
}

} // namespace Common
} // namespace NFE
