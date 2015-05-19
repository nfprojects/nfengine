/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with class representing VFS file from file on disk
 */

#pragma once

#include "../nfCommon.hpp"

#include "PackerResource.hpp"

namespace NFE {
namespace Common {

class PackerResourceFile: public PackerResource
{
public:
    PackerResult Init(const std::string& filePath, const std::string& vfsFilePath);
    PackerResult Save(File* file);

private:
    std::string mFilePath;
};

} // namespace Common
} // namespace NFE
