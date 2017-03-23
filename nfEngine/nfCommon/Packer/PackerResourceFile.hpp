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
    ~PackerResourceFile();

    PackerResult Init(const String& filePath, const String& vfsFilePath);
    PackerResult Save(File& file);

private:
    String mFilePath;
};

} // namespace Common
} // namespace NFE
