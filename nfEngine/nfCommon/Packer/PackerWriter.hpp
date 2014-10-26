/**
 * @file   PackerWriter.hpp
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with class used to write files to PAK archive.
 */

#pragma once

#include "PackerDefines.hpp"
#include "PackerElement.hpp"

namespace NFE {
namespace Common {

class NFCOMMON_API PackWriter
{
public:
    PackWriter();
    PackWriter(const std::string& archiveName);

    PackResult Init(const std::string& archiveName);

    PackResult AddFile(const std::string& filePath, const std::string& vfsFilePath);
    PackResult AddFile(const Buffer& buffer, const std::string vfsFilePath);
    PackResult AddFilesRecursively(const std::string& startPath);
    PackResult WritePAK() const;

    void PrintFilesToStdout() const;
    const FileListType& GetFiles() const;
    size_t GetFileCount() const;
    const std::string& GetPAKName() const;

private:
    std::string mFilePath;
    FileListType mFileList;
};

} // namespace Common
} // namespace NFE
