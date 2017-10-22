/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with class used to write files to PAK archive.
 */

#pragma once

#include "PackerDefines.hpp"
#include "PackerResourceFile.hpp"
#include "../Containers/String.hpp"

namespace NFE {
namespace Common {

class NFCOMMON_API PackerWriter
{
public:
    PackerWriter();
    PackerWriter(const StringView archiveName);

    PackerResult Init(const StringView archiveName);

    PackerResult AddFile(const StringView filePath, const StringView vfsFilePath);
    PackerResult AddFile(const Buffer& buffer, const StringView vfsFilePath);
    PackerResult AddFilesRecursively(const StringView startPath);
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
