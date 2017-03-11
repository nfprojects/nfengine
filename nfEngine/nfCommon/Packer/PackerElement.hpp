/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with class representing read VFS file.
 */

#pragma once

#include "PackerDefines.hpp"
#include "../Utils/MD5.hpp"


namespace NFE {
namespace Common {

struct PackerElement
{
    MD5Hash mHash;
    size_t mFilePos;
    size_t mFileSize;
};

typedef std::vector<PackerElement> VFSFileListType;

} // namespace Common
} // namespace NFE
