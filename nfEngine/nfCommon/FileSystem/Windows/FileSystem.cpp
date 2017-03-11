/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Windows implementation of filesystem utilities.
 */

#include "PCH.hpp"
#include "../FileSystem.hpp"
#include "Logger/Logger.hpp"
#include "System/Win/Common.hpp"

#include <stack>

namespace NFE {
namespace Common {

namespace {

static const std::wstring allFilesWildcard = L"\\*";

bool RecursiveDeleteDirectory(const std::wstring& path)
{
    WIN32_FIND_DATA findData;

    std::wstring spec = path + allFilesWildcard;
    HANDLE findHandle = FindFirstFile(spec.c_str(), &findData);
    if (findHandle == INVALID_HANDLE_VALUE)
    {
        LOG_ERROR("FindFirstFile failed for path '%s': %s", path.c_str(),
                  GetLastErrorString().c_str());
        return false;
    }

    if (findHandle != INVALID_HANDLE_VALUE)
    {
        for (;;)
        {
            // ignore special paths
            if (wcscmp(findData.cFileName, L".") != 0 && wcscmp(findData.cFileName, L"..") != 0)
            {
                bool isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

                // construct full path
                std::wstring filePath = path + L'\\' + findData.cFileName;

                // remove the file or directory recursively
                if (isDirectory)
                    RecursiveDeleteDirectory(filePath);
                else
                {
                    if (::DeleteFile(filePath.c_str()) == FALSE)
                    {
                        std::string shortPath;
                        if (UTF16ToUTF8(filePath, shortPath))
                        {
                            LOG_ERROR("Failed to delete file '%s': %s", shortPath.c_str(),
                                      GetLastErrorString().c_str());
                        }
                    }
                }
            }

            // search for the next object
            if (::FindNextFile(findHandle, &findData) == FALSE)
            {
                DWORD lastError = ::GetLastError();
                if (lastError != ERROR_NO_MORE_FILES)
                    LOG_ERROR("FindNextFile() failed: %s", GetLastErrorString().c_str());
                break;
            }
        }

        FindClose(findHandle);
    }

    // now we can remove empty directory
    if (::RemoveDirectory(path.c_str()) == FALSE)
    {
        std::string shortPath;
        std::string error = GetLastErrorString();
        if (UTF16ToUTF8(path, shortPath))
        {
            LOG_ERROR("Failed to remove directory '%s': %s", shortPath.c_str(), error.c_str());
        }
        return false;
    }

    return true;
}

} // namespace

std::string FileSystem::GetExecutablePath()
{
    std::unique_ptr<TCHAR[]> execPath;
    std::string execPathStr;
    DWORD sizeRead = 0;
    unsigned int len = MAX_PATH; // Maximum length of a relative paths, available in Windows
    const unsigned int maxPathWide = 32768; // Maximum length of a path, available in Windows

    for (; len < maxPathWide; len *= 2)
    {
        execPath.reset(new TCHAR[len]);
        sizeRead = GetModuleFileName(nullptr, execPath.get(), len);

        if (sizeRead < len && sizeRead != 0)
            break;
    }

    // Check if the buffer did not overflow, if not - convert to UTF8 and check result
    if (len >= maxPathWide)
    {
        LOG_ERROR("Failed to resolve executable's path : %s", GetLastErrorString().c_str());
        return "";
    }
    if (!UTF16ToUTF8(execPath.get(), execPathStr))
    {
        LOG_ERROR("UTF conversion of executable's path failed : %s", GetLastErrorString().c_str());
        return "";
    }

    return execPathStr;
}

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

bool FileSystem::TouchFile(const std::string& path)
{
    std::wstring widePath;
    if (!UTF8ToUTF16(path, widePath))
        return false;

    HANDLE fileHandle = ::CreateFile(widePath.c_str(), 0, 0, NULL, CREATE_NEW,
                                     FILE_ATTRIBUTE_NORMAL, 0);
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        LOG_ERROR("Failed to create file '%s': %s", path.c_str(), GetLastErrorString().c_str());
        return false;
    }

    CloseHandle(fileHandle);
    return true;
}

PathType FileSystem::GetPathType(const std::string& path)
{
    std::wstring widePath;
    if (!UTF8ToUTF16(path, widePath))
        return PathType::Invalid;

    DWORD attrs = GetFileAttributes(widePath.c_str());

    if (attrs == INVALID_FILE_ATTRIBUTES)
        return PathType::Invalid;
    if (attrs & FILE_ATTRIBUTE_DIRECTORY)
        return PathType::Directory;

    return PathType::File;
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

    DWORD attrs = GetFileAttributes(widePath.c_str());
    if (INVALID_FILE_ATTRIBUTES == attrs)
    {
        LOG_INFO("Failed to retrieve attributes for path '%s': %s", path.c_str(),
                 GetLastErrorString().c_str());
        return false;
    }

    if (attrs & FILE_ATTRIBUTE_DIRECTORY)
    {
        if (recursive)
        {
            if (!RecursiveDeleteDirectory(widePath))
                return false;
        }
        else if (::RemoveDirectory(widePath.c_str()) == FALSE)
        {
            LOG_ERROR("Failed to remove '%s': %s", path.c_str(),
                      GetLastErrorString().c_str());
            return false;
        }
    }
    else // delete file only
    {
        BOOL result = ::DeleteFile(widePath.c_str());
        if (result == FALSE)
        {
            LOG_ERROR("Failed to delete file '%s': %s", path.c_str(),
                      GetLastErrorString().c_str());
            return false;
        }
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

bool FileSystem::Iterate(const std::string& path, DirIterateCallback callback)
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
        spec = widePath + allFilesWildcard;
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

            // ignore multiple '/' or '\'
            // TODO create and use function for path normalization
            while (widePath.back() == L'/' || widePath.back() == L'\\')
                widePath.pop_back();

            foundPath = widePath + L'/' + findData.cFileName;
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
            std::string newPath;
            if (UTF16ToUTF8(foundPath, newPath))
            {
                if (!callback(newPath, isDir))
                {
                    FindClose(findHandle);
                    return true;
                }
            }
        }
        while (FindNextFile(findHandle, &findData) != 0);

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
