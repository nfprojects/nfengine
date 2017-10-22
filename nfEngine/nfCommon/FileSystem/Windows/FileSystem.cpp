/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Windows implementation of filesystem utilities.
 */

#include "PCH.hpp"
#include "../FileSystem.hpp"
#include "Logger/Logger.hpp"
#include "System/Win/Common.hpp"
#include "../../Containers/UniquePtr.hpp"
#include "../../Containers/DynArray.hpp"

#include <stack>

namespace NFE {
namespace Common {

namespace {

static const Utf16String allFilesWildcard = L"\\*";

bool RecursiveDeleteDirectory(const Utf16String& path)
{
    WIN32_FIND_DATA findData;

    Utf16String spec = path + allFilesWildcard;

    HANDLE findHandle = FindFirstFile(spec.c_str(), &findData);
    if (findHandle == INVALID_HANDLE_VALUE)
    {
        NFE_LOG_ERROR("FindFirstFile failed: %s", GetLastErrorString().Str());
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
                Utf16String filePath = path + L'\\' + findData.cFileName;

                // remove the file or directory recursively
                if (isDirectory)
                    RecursiveDeleteDirectory(filePath);
                else
                {
                    if (::DeleteFile(filePath.c_str()) == FALSE)
                    {
                        String shortPath;
                        if (UTF16ToUTF8(filePath, shortPath))
                        {
                            NFE_LOG_ERROR("Failed to delete file '%s': %s", shortPath.Str(), GetLastErrorString().Str());
                        }
                    }
                }
            }

            // search for the next object
            if (::FindNextFile(findHandle, &findData) == FALSE)
            {
                DWORD lastError = ::GetLastError();
                if (lastError != ERROR_NO_MORE_FILES)
                {
                    NFE_LOG_ERROR("FindNextFile() failed: %s", GetLastErrorString().Str());
                }
                break;
            }
        }

        FindClose(findHandle);
    }

    // now we can remove empty directory
    if (::RemoveDirectory(path.c_str()) == FALSE)
    {
        String shortPath;
        String error = GetLastErrorString();
        if (UTF16ToUTF8(path, shortPath))
        {
            NFE_LOG_ERROR("Failed to remove directory '%s': %s", shortPath.Str(), error.Str());
        }
        return false;
    }

    return true;
}

} // namespace

String FileSystem::GetExecutablePath()
{
    DynArray<wchar_t> execPath;

    DWORD sizeRead = 0;
    unsigned int len = MAX_PATH; // Maximum length of a relative paths, available in Windows
    const unsigned int maxPathWide = 32768; // Maximum length of a path, available in Windows

    for (; len < maxPathWide; len *= 2)
    {
        execPath.Resize(len);
        sizeRead = GetModuleFileName(nullptr, execPath.Data(), len);

        if (sizeRead < len && sizeRead != 0)
            break;
    }

    // Check if the buffer did not overflow, if not - convert to UTF8 and check result
    if (len >= maxPathWide)
    {
        NFE_LOG_ERROR("Failed to resolve executable's path : %s", GetLastErrorString().Str());
        return "";
    }

    String execPathStr;
    if (!UTF16ToUTF8(Utf16String(execPath.Data()), execPathStr))
    {
        NFE_LOG_ERROR("UTF conversion of executable's path failed : %s", GetLastErrorString().Str());
        return "";
    }

    return execPathStr;
}

bool FileSystem::ChangeDirectory(const StringView path)
{
    Utf16String widePath;
    if (!UTF8ToUTF16(path, widePath))
        return false;

    if (::SetCurrentDirectory(widePath.c_str()) == 0)
    {
        NFE_LOG_ERROR("Failed to change directory to '%.*s': %s", path.Length(), path.Data(), GetLastErrorString().Str());
        return false;
    }

    NFE_LOG_INFO("Current directory changed to: '%.*s'", path.Length(), path.Data());
    return true;
}

bool FileSystem::TouchFile(const StringView path)
{
    Utf16String widePath;
    if (!UTF8ToUTF16(path, widePath))
        return false;

    HANDLE fileHandle = ::CreateFile(widePath.c_str(), 0, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        NFE_LOG_ERROR("Failed to create file '%.*s': %s", path.Length(), path.Data(), GetLastErrorString().Str());
        return false;
    }

    CloseHandle(fileHandle);
    return true;
}

PathType FileSystem::GetPathType(const StringView path)
{
    Utf16String widePath;
    if (!UTF8ToUTF16(path, widePath))
        return PathType::Invalid;

    const DWORD attrs = GetFileAttributes(widePath.c_str());

    if (attrs == INVALID_FILE_ATTRIBUTES)
        return PathType::Invalid;
    if (attrs & FILE_ATTRIBUTE_DIRECTORY)
        return PathType::Directory;

    return PathType::File;
}

bool FileSystem::CreateDir(const StringView path)
{
    Utf16String widePath;
    if (!UTF8ToUTF16(path, widePath))
        return false;

    if (::CreateDirectory(widePath.c_str(), nullptr) == 0)
    {
        NFE_LOG_ERROR("Failed to create directory '%.*s': %s", path.Length(), path.Data(), GetLastErrorString().Str());
        return false;
    }

    NFE_LOG_INFO("Created directory '%.*s'", path.Length(), path.Data());
    return true;
}

bool FileSystem::Remove(const StringView path, bool recursive)
{
    Utf16String widePath;
    if (!UTF8ToUTF16(path, widePath))
        return false;

    const DWORD attrs = GetFileAttributes(widePath.c_str());
    if (INVALID_FILE_ATTRIBUTES == attrs)
    {
        NFE_LOG_INFO("Failed to retrieve attributes for path '%.*s': %s", path.Length(), path.Data(), GetLastErrorString().Str());
        return false;
    }

    if (attrs & FILE_ATTRIBUTE_DIRECTORY)
    {
        if (recursive)
        {
            if (!RecursiveDeleteDirectory(widePath))
            {
                return false;
            }
        }
        else if (::RemoveDirectory(widePath.c_str()) == FALSE)
        {
            NFE_LOG_ERROR("Failed to remove '%.*s': %s", path.Length(), path.Data(), GetLastErrorString().Str());
            return false;
        }
    }
    else // delete file only
    {
        BOOL result = ::DeleteFile(widePath.c_str());
        if (result == FALSE)
        {
            NFE_LOG_ERROR("Failed to delete file '%.*s': %s", path.Length(), path.Data(), GetLastErrorString().Str());
            return false;
        }
    }

    NFE_LOG_INFO("Removed '%.*s'", path.Length(), path.Data());
    return true;
}

bool FileSystem::Copy(const StringView srcPath, const StringView destPath, bool overwrite)
{
    Utf16String wideSrcPath, wideDestPath;
    if (!UTF8ToUTF16(srcPath, wideSrcPath) || !UTF8ToUTF16(destPath, wideDestPath))
        return false;

    if (::CopyFile(wideSrcPath.c_str(), wideDestPath.c_str(), !overwrite) == 0)
    {
        NFE_LOG_ERROR("Failed to copy file '%.*s' to '%.*s': %s",
            srcPath.Length(), srcPath.Data(), destPath.Length(), destPath.Data(),
            GetLastErrorString().Str());
        return false;
    }

    NFE_LOG_INFO("File '%.*s' copied to '%.*s'", srcPath.Length(), srcPath.Data(), destPath.Length(), destPath.Data());
    return true;
}

bool FileSystem::Move(const StringView srcPath, const StringView destPath)
{
    Utf16String wideSrcPath, wideDestPath;
    if (!UTF8ToUTF16(srcPath, wideSrcPath) || !UTF8ToUTF16(destPath, wideDestPath))
        return false;

    if (::MoveFile(wideSrcPath.c_str(), wideDestPath.c_str()) == 0)
    {
        NFE_LOG_ERROR("Failed to move file '%.*s' to '%s': %.*s", srcPath.Length(), srcPath.Data(), destPath.Length(), destPath.Data(), GetLastErrorString().Str());
        return false;
    }

    NFE_LOG_INFO("File '%.*s' moved to '%.*s'", srcPath.Length(), srcPath.Data(), destPath.Length(), destPath.Data());
    return true;
}

bool FileSystem::Iterate(const StringView path, const DirIterateCallback& callback)
{
    Utf16String widePath;
    HANDLE findHandle = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA findData;
    Utf16String spec, foundPath;
    std::stack<Utf16String> directories;

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
            NFE_LOG_ERROR("FindFirstFile failed for path '%.*s': %s", path.Length(), path.Data(), GetLastErrorString().Str());
            return false;
        }

        // find all elements in current node directory
        do
        {
            // ignore special paths
            if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0)
            {
                continue;
            }

            // ignore multiple '/' or '\'
            // TODO create and use function for path normalization
            while (widePath.back() == L'/' || widePath.back() == L'\\')
            {
                widePath.pop_back();
            }

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
            String newPath;
            if (UTF16ToUTF8(foundPath, newPath))
            {
                if (!callback(newPath, isDir ? PathType::Directory : PathType::File))
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
            NFE_LOG_ERROR("FindNextFile failed for path '%.*s': %s", path.Length(), path.Data(), GetLastErrorString().Str());
            return false;
        }

        FindClose(findHandle);
        findHandle = INVALID_HANDLE_VALUE;
    }

    return true;
}

} // namespace Common
} // namespace NFE
