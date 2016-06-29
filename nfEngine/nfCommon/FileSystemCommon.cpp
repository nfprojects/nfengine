/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Common filesystem functions.
 */

#include "PCH.hpp"
#include "FileSystem.hpp"

namespace NFE {
namespace Common {

std::string FileSystem::GetParentDir(const std::string& path)
{
    size_t found = path.find_last_of("/\\");
    return path.substr(0, found);
}

std::string FileSystem::ExtractExtension(const std::string& path)
{
    return path.substr(path.find_last_of('.') + 1);
}

bool FileSystem::CreateDirIfNotExist(const std::string& path)
{
    if (FileSystem::GetPathType(path) != PathType::Directory)
        return FileSystem::CreateDir(path);
    return true;
}


} // namespace Common
} // namespace NFE
