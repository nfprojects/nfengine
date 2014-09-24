/**
    NFEngine project

    \file   packerWriter.hpp
    \author LKostyra (costyrra.xl@gmail.com)
    \brief  Module with class used to write files to PAK archive.
*/

#pragma once

#include "packerDefines.hpp"
#include "packerElement.hpp"

namespace NFE {
namespace Common {

class PackWriter
{
public:
    NFCOMMON_API PackWriter();
    NFCOMMON_API PackWriter(const std::string& ArchiveName);

    NFCOMMON_API PACK_RESULT Init(const std::string& ArchiveName);

    NFCOMMON_API PACK_RESULT AddFile(const std::string& FilePath, const std::string& VFSFilePath);
    NFCOMMON_API PACK_RESULT AddFilesRecursively(const std::string& StartPath);
    NFCOMMON_API PACK_RESULT WritePAK() const;

    NFCOMMON_API void PrintFilesToStdout() const;
    NFCOMMON_API const FileListType& GetFiles() const;
    NFCOMMON_API size_t GetFileCount() const;
    NFCOMMON_API const std::string& GetPAKName() const;

#ifdef NFCOMMON_EXPORTS
private:
    std::string mFilePath;
    FileListType mFileList;
#endif
};

} // namespace Common
} // namespace NFE
