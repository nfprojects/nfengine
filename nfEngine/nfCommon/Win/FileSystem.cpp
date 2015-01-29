/**
 * @file   FileSystem.cpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Windows implementation of filesystem utilities.
 */

#include "../stdafx.hpp"
#include "../FileSystem.hpp"
#include "../Logger.hpp"

#include "Common.hpp"

// TODO: move to precompiled header
#include <stack>

namespace NFE {
namespace Common {

using namespace Windows;

bool FileSystem::ChangeDirectory(const std::string& path)
{
    std::wstring widePath;
    if (!UTF8ToUTF16(path, widePath))
        return false;

    if (::SetCurrentDirectory(widePath.c_str()) == 0)
    {
        LOG_ERROR("Failed to change directory to '%s': %s", path.c_str(),
                  GetLastErrorString().c_str());
        return false;
    }

    LOG_INFO("Current directory changed to: '%s'", path.c_str());
    return true;
}

bool FileSystem::CreateDir(const std::string& path)
{
    std::wstring widePath;
    if (!UTF8ToUTF16(path, widePath))
        return false;

    if (::CreateDirectory(widePath.c_str(), nullptr) == 0)
    {
        LOG_ERROR("Failed to create directory '%s': %s", path.c_str(),
                  GetLastErrorString().c_str());
        return false;
    }

    LOG_INFO("Created directory '%s'", path.c_str());
    return true;
}

bool FileSystem::Remove(const std::string& path, bool recursive)
{
    std::wstring widePath;
    if (!UTF8ToUTF16(path, widePath))
        return false;

    // TODO...

    if (::RemoveDirectory(widePath.c_str()) != 0)
    {
        LOG_ERROR("Failed to remove '%s': %s", path.c_str(),
                  GetLastErrorString().c_str());
        return false;
    }

    LOG_INFO("Removed '%s'", path.c_str());
    return true;
}

bool FileSystem::Copy(const std::string& srcPath, const std::string& destPath, bool overwrite)
{
    std::wstring wideSrcPath, wideDestPath;
    if (!UTF8ToUTF16(srcPath, wideSrcPath) || !UTF8ToUTF16(destPath, wideDestPath))
        return false;

    if (::CopyFile(wideSrcPath.c_str(), wideDestPath.c_str(), !overwrite) == 0)
    {
        LOG_ERROR("Failed to copy file '%s' to '%s': %s", srcPath.c_str(), destPath.c_str(),
                  GetLastErrorString().c_str());
        return false;
    }

    LOG_INFO("File '%s' copied to '%s'", srcPath.c_str(), destPath.c_str());
    return true;
}

bool FileSystem::Move(const std::string& srcPath, const std::string& destPath)
{
    std::wstring wideSrcPath, wideDestPath;
    if (!UTF8ToUTF16(srcPath, wideSrcPath) || !UTF8ToUTF16(destPath, wideDestPath))
        return false;

    if (::MoveFile(wideSrcPath.c_str(), wideDestPath.c_str()) == 0)
    {
        LOG_ERROR("Failed to move file '%s' to '%s': %s", srcPath.c_str(), destPath.c_str(),
                  GetLastErrorString().c_str());
        return false;
    }

    LOG_INFO("File '%s' moved to '%s'", srcPath.c_str(), destPath.c_str());
    return true;
}

bool FileSystem::List(const std::string& path, DirListCallback callback)
{
    std::wstring widePath;
    HANDLE findHandle = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA findData;
    std::wstring spec, foundPath;
    std::stack<std::wstring> directories;

    if (!UTF8ToUTF16(path, widePath))
        return false;

    directories.push(widePath);

    while (!directories.empty())
    {
        widePath = directories.top();
        spec = widePath + L"\\*";
        directories.pop();

        findHandle = FindFirstFile(spec.c_str(), &findData);
        if (findHandle == INVALID_HANDLE_VALUE)
        {
            LOG_ERROR("FindFirstFile failed for path '%s': %s", path.c_str(),
                      GetLastErrorString().c_str());
            return false;
        }

        // find all elements in current node directory
        do
        {
            // ignore special paths
            if (wcscmp(findData.cFileName, L".") == 0 ||
                wcscmp(findData.cFileName, L"..") == 0)
                continue;

            foundPath = widePath + L'\\' + findData.cFileName;
            bool isDir = false;

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                directories.push(foundPath);
                isDir = true;
            }

            /*
             * TODO:
             * At the moment whole path is converted from UTF-16 to UTF-8, which is inefficient
             * for directories with long name and many files inside it.
             * Consider converting only findData.cFileName. This will require keeping both
             * UTF-8 and UTF-16 versions of directories paths on the stack.
             */
            std::string path;
            if (UTF16ToUTF8(foundPath, path))
            {
                if (!callback(path, isDir))
                {
                    FindClose(findHandle);
                    return true;
                }
            }
        } while (FindNextFile(findHandle, &findData) != 0);

        if (GetLastError() != ERROR_NO_MORE_FILES)
        {
            FindClose(findHandle);
            LOG_ERROR("FindNextFile failed for path '%s': %s", path.c_str(),
                      GetLastErrorString().c_str());
            return false;
        }

        FindClose(findHandle);
        findHandle = INVALID_HANDLE_VALUE;
    }

    return true;
}

} // namespace Common
} // namespace NFE
