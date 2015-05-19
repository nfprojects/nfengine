/**
 * @file   PackerWriter.hpp
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with class used to write files to PAK archive.
 */

#pragma once

#include "PackerDefines.hpp"
#include "PackerResourceFile.hpp"

namespace NFE {
namespace Common {

class NFCOMMON_API PackerWriter
{
public:
    PackerWriter();
    PackerWriter(const std::string& archiveName);

    PackerResult Init(const std::string& archiveName);

    PackerResult AddFile(const std::string& filePath, const std::string& vfsFilePath);
    PackerResult AddFile(const Buffer& buffer, const std::string& vfsFilePath);
    PackerResult AddFilesRecursively(const std::string& startPath);
    PackerResult WritePAK() const;

    void PrintFilesToStdout() const;
    const ResourceListType& GetFiles() const;
    size_t GetFileCount() const;
    const std::string& GetPAKName() const;

private:
    std::string mFilePath;
    ResourceListType mFileList;
};

} // namespace Common
} // namespace NFE
