/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with abstract class representing single VFS file.
 */

#pragma once

#include "PackerDefines.hpp"
#include "../MD5.hpp"

#include <memory>


namespace NFE {
namespace Common {

class PackerResource
{
public:
    virtual ~PackerResource();

    virtual PackerResult Save(File& file) = 0;

    PackerResult SaveHeader(File& file);

    size_t GetFileSize() const;
    void PrintToStdout() const;

protected:
    MD5Hash mHash;
    size_t mFileSize;
};

typedef std::vector<std::shared_ptr<PackerResource>> ResourceListType;

} // namespace Common
} // namespace NFE
