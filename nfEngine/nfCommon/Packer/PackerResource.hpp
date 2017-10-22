/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with abstract class representing single VFS file.
 */

#pragma once

#include "PackerDefines.hpp"
#include "../Utils/MD5.hpp"
#include "../Containers/DynArray.hpp"
#include "../Containers/SharedPtr.hpp"


namespace NFE {
namespace Common {

class PackerResource
{
public:
    virtual ~PackerResource();

    virtual PackerResult Save(File& file) = 0;

    PackerResult SaveHeader(File& file);

    uint32 GetFileSize() const;
    void PrintToStdout() const;

protected:
    MD5Hash mHash;
    uint32 mFileSize;
};

typedef DynArray<SharedPtr<PackerResource>> ResourceListType;

} // namespace Common
} // namespace NFE
