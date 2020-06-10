/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Common filesystem functions.
 */

#include "PCH.hpp"
#include "FileSystem.hpp"

namespace NFE {
namespace Common {


StringView FileSystem::GetParentDir(const StringView& path)
{
    for (uint32 i = path.Length(); i-- > 0; )
    {
        if (path[i] == '/' || path[i] == '\\')
        {
            if (i != path.Length() - 1)
            {
                return path.Range(0, i);
            }
        }
     }

    // path separator not found
    return StringView();
}

StringView FileSystem::GetExtension(const StringView& path)
{
    for (uint32 i = path.Length(); i-- > 0; )
    {
        if (path[i] == '/' || path[i] == '\\')
        {
            break;
        }
        else if (path[i] == '.')
        {
            if (i + 1 < path.Length())
            {
                return path.Range(i + 1, path.Length() - i - 1);
            }
        }
    }

    // extension not found
    return StringView();
}

bool FileSystem::CreateDirIfNotExist(const StringView& path)
{
    if (FileSystem::GetPathType(path) != PathType::Directory)
    {
        return FileSystem::CreateDir(path);
    }

    return true;
}


} // namespace Common
} // namespace NFE
