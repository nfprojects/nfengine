/**
 * @file   PackerResource.cpp
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with abstract class representing single VFS file.
 */

#include "../stdafx.hpp"

#include "PackerResource.hpp"

namespace NFE {
namespace Common {

PackerResult PackerResource::SaveHeader(FILE* file)
{
    if (!fwrite(reinterpret_cast<const void*>(&mHash), sizeof(uint32), 4, file))
        return PackerResult::WriteFailed;

    if (!fwrite(reinterpret_cast<const void*>(&mFileSize), sizeof(size_t), 1, file))
        return PackerResult::WriteFailed;

    return PackerResult::OK;
}

const size_t PackerResource::GetFileSize() const
{
    return mFileSize;
}

void PackerResource::PrintToStdout() const
{
    std::cout << "Element " << mHash << ", size " << mFileSize << std::endl;
}

} // namespace Common
} // namespace NFE
