/**
    NFEngine project

    \file   packerReader.hpp
    \author LKostyra (costyrra.xl@gmail.com)
    \brief  Module with class used to read files from PAK archive.
*/

#pragma once

#include "packerDefines.hpp"
#include "packerElement.hpp"

#include "../Buffer.h"

namespace NFE {
namespace Common {

class PackReader
{
public:
    NFCOMMON_API PackReader();
    NFCOMMON_API PackReader(const std::string& FilePath);

    NFCOMMON_API PACK_RESULT Init(const std::string& FilePath);
    NFCOMMON_API PACK_RESULT GetFile(const std::string& VFSFilePath, Buffer& outbuf);

    NFCOMMON_API void PrintFilesToStdout() const;
    NFCOMMON_API size_t GetFileCount() const;
    NFCOMMON_API uint32 GetFileVersion() const;

#ifdef NFCOMMON_EXPORTS
private:
    uint32 mFileVersion;
    std::string mFilePath;
    FileListType mFileList;
#endif
};

} // namespace Common
} // namespace NFE
