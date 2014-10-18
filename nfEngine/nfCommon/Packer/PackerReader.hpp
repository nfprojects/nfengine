/**
 * @file   PackerReader.hpp
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with class used to read files from PAK archive.
 */

#pragma once

#include "PackerDefines.hpp"
#include "PackerElement.hpp"

#include "../Buffer.hpp"

namespace NFE {
namespace Common {

class NFCOMMON_API PackReader
{
public:
    PackReader();
    PackReader(const std::string& filePath);

    PackResult Init(const std::string& filePath);
    PackResult GetFile(const std::string& vfsFilePath, Buffer& outputBuffer);

    void PrintFilesToStdout() const;
    size_t GetFileCount() const;
    uint32 GetFileVersion() const;

private:
    uint32 mFileVersion;
    std::string mFilePath;
    FileListType mFileList;
};

} // namespace Common
} // namespace NFE
