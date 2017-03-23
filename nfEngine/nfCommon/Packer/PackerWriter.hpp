/**
 * @file
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
    PackerWriter(const String& archiveName);

    PackerResult Init(const String& archiveName);

    PackerResult AddFile(const String& filePath, const String& vfsFilePath);
    PackerResult AddFile(const Buffer& buffer, const String& vfsFilePath);
    PackerResult AddFilesRecursively(const String& startPath);
    PackerResult WritePAK() const;

    void PrintFilesToStdout() const;
    const ResourceListType& GetFiles() const;
    size_t GetFileCount() const;
    const String& GetPAKName() const;

private:
    String mFilePath;
    ResourceListType mFileList;
};

} // namespace Common
} // namespace NFE
