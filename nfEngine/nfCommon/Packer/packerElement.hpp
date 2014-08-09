/**
    NFEngine project

    \file   packerElement.h
    \author LKostyra (costyrra.xl@gmail.com)
    \brief  Module with class representing single file.
*/

#pragma once

#include "packerDefines.hpp"
#include "../MD5.h"

namespace NFE {
namespace Common {

struct packElement
{
    MD5Hash mHash;
    std::string FilePath;
    long FilePos;
    size_t FileSize;
};

typedef std::vector<packElement> FileListType;

} // namespace Common
} // namespace NFE
