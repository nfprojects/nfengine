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

// TODO: For now PackerElement is written to hold only one type of resource - file in filesystem.
//       To prepare PackerElement for holding more types of resources, PackerElement needs to be
//       rewritten to be an abstract class and inherited classes must be made for each type of
//       resource to be handled by Packer.
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
