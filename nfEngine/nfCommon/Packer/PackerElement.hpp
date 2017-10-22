/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with class representing read VFS file.
 */

#pragma once

#include "PackerDefines.hpp"
#include "../Utils/MD5.hpp"
#include "../Containers/DynArray.hpp"

namespace NFE {
namespace Common {

struct PackerElement
{
    MD5Hash mHash;
    uint64 mFilePos;
    uint32 mFileSize;
};

typedef DynArray<PackerElement> VFSFileListType;

} // namespace Common
} // namespace NFE
