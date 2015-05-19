/**
 * @file   PackerReader.hpp
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with class used to read files from PAK archive.
 */

#pragma once

#include "PackerDefines.hpp"
#include "PackerElement.hpp"

namespace NFE {
namespace Common {

class NFCOMMON_API PackerReader
{
public:
    PackerReader();
    PackerReader(const std::string& filePath);

    PackerResult Init(const std::string& filePath);
    PackerResult GetFile(const std::string& vfsFilePath, Buffer& outputBuffer);

    void PrintFilesToStdout() const;
    size_t GetFileCount() const;
    uint32 GetFileVersion() const;

private:
    uint32 mFileVersion;
    std::string mFilePath;
    VFSFileListType mFileList;
};

} // namespace Common
} // namespace NFE
