/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with class used to read files from PAK archive.
 */

#pragma once

#include "PackerDefines.hpp"
#include "PackerElement.hpp"

#include "../Containers/StringView.hpp"
#include "../Containers/String.hpp"


namespace NFE {
namespace Common {

class NFCOMMON_API PackerReader
{
public:
    PackerReader();
    PackerReader(const StringView filePath);

    PackerResult Init(const StringView filePath);
    PackerResult GetFile(const StringView vfsFilePath, Buffer& outputBuffer);

    void PrintFilesToStdout() const;
    uint32 GetFileCount() const;
    uint32 GetFileVersion() const;

private:
    uint32 mFileVersion;
    String mFilePath;
    VFSFileListType mFileList;
};

} // namespace Common
} // namespace NFE
