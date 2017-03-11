/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module with abstract class representing single VFS file.
 */

#include "PCH.hpp"

#include "PackerResource.hpp"

namespace NFE {
namespace Common {

PackerResource::~PackerResource()
{
}

PackerResult PackerResource::SaveHeader(File& file)
{
    if (!file.Write(reinterpret_cast<const void*>(&mHash), 4 * sizeof(uint32)))
        return PackerResult::WriteFailed;

    if (!file.Write(reinterpret_cast<const void*>(&mFileSize), sizeof(size_t)))
        return PackerResult::WriteFailed;

    return PackerResult::OK;
}

size_t PackerResource::GetFileSize() const
{
    return mFileSize;
}

void PackerResource::PrintToStdout() const
{
    std::cout << "Element " << mHash << ", size " << mFileSize << std::endl;
}

} // namespace Common
} // namespace NFE
