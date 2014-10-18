/**
    NFEngine project

    \file   packerElement.hpp
    \author LKostyra (costyrra.xl@gmail.com)
    \brief  Module with class representing single file.
*/

#pragma once

#include "packerDefines.hpp"
#include "../MD5.hpp"

namespace NFE {
namespace Common {

struct packElement
{
    MD5Hash mHash;
    std::string FilePath;
    size_t FilePos;
    size_t FileSize;
};

typedef std::vector<packElement> FileListType;

} // namespace Common
} // namespace NFE
