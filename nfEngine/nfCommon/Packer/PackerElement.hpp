/**
 * @file   PackerElement.hpp
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with class representing single file.
 */

#pragma once

#include "PackerDefines.hpp"
#include "../MD5.hpp"

namespace NFE {
namespace Common {

struct PackerElement
{
    MD5Hash mHash;
    std::string FilePath;
    size_t FilePos;
    size_t FileSize;
};

typedef std::vector<PackerElement> FileListType;

} // namespace Common
} // namespace NFE
