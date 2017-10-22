/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Common filesystem functions.
 */

#include "PCH.hpp"
#include "FileSystem.hpp"

namespace NFE {
namespace Common {


StringView FileSystem::GetParentDir(const StringView path)
{
    uint32 index = path.FindLast('/');
    if (index == std::numeric_limits<uint32>::max())
    {
        index = path.FindLast('\\');
    }

    if (index != std::numeric_limits<uint32>::max())
    {
        return path.Range(0, index);
    }

    // path separator not found
    return StringView();
}

StringView FileSystem::ExtractExtension(const StringView path)
{
    const uint32 index = path.FindLast('.');
    if (index != std::numeric_limits<uint32>::max())
    {
        return path.Range(index, path.Length() - index);
    }

    // extension not found
    return StringView();
}

bool FileSystem::CreateDirIfNotExist(const StringView path)
{
    if (FileSystem::GetPathType(path) != PathType::Directory)
    {
        return FileSystem::CreateDir(path);
    }

    return true;
}


} // namespace Common
} // namespace NFE
