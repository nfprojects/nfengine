/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Linux implementation of filesystem utilities.
 */

#include "PCH.hpp"
#include "../FileSystem.hpp"
#include "Logger/Logger.hpp"
#include "Containers/String.hpp"
#include "Containers/StringView.hpp"
#include "Containers/Deque.hpp"

#include <dirent.h>


namespace NFE {
namespace Common {

namespace {

bool RecursiveDeleteDirectory(const StringView& path)
{
     const StringViewToCStringHelper pathString(path);

    DIR* d;
    struct dirent* dir;
    String foundPath;

    d = ::opendir(pathString);
    if (d == NULL)
    {
        NFE_LOG_ERROR("opendir() failed for path '%s': %s", pathString.Str(), strerror(errno));
        return false;
    }

    // find all elements in current node directory
    while ((dir = ::readdir(d)) != NULL)
    {
        // ignore special paths
        if (::strcmp(dir->d_name, ".") == 0 || ::strcmp(dir->d_name, "..") == 0)
            continue;

        foundPath = path + '/' + dir->d_name;
        bool isDir = dir->d_type == DT_DIR;

        if (isDir)
            RecursiveDeleteDirectory(foundPath);
        else if (::unlink(foundPath.Str()) != 0)
            NFE_LOG_ERROR("Failed to delete file '%s': %s", foundPath.Str(), strerror(errno));
    }

    ::closedir(d);

    // now we can remove empty directory
    if (::rmdir(pathString) != 0)
    {
        NFE_LOG_ERROR("Failed to remove directory '%s': %s", pathString.Str(), strerror(errno));
        return false;
    }

    return true;
}

} // namespace

String FileSystem::GetExecutablePath()
{
    String linkPath("/proc/self/exe");
    String execPathStr;
    char* execPath = realpath(linkPath.Str(), nullptr);

    if (!execPath)
        NFE_LOG_ERROR("Failed to resolve executable's path : %s", strerror(errno));
    else
    {
        execPathStr = execPath;
        free(execPath);
    }

    return execPathStr;
}

bool FileSystem::ChangeDirectory(const StringView& path)
{
    const StringViewToCStringHelper pathString(path);

    if (::chdir(pathString) != 0)
    {
        NFE_LOG_ERROR("Failed to change current directory to '%s': %s", pathString.Str(), strerror(errno));
        return false;
    }

    NFE_LOG_INFO("Current directory changed to: '%s'", pathString.Str());
    return true;
}

bool FileSystem::TouchFile(const StringView& path)
{
    const StringViewToCStringHelper pathString(path);

    int fd = ::open(pathString, O_WRONLY | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);

    if (fd < 0)
    {
        NFE_LOG_ERROR("Failed to create file '%s': %s", pathString.Str(), strerror(errno));
        return false;
    }

    ::close(fd);
    return true;
}

PathType FileSystem::GetPathType(const StringView& path)
{
    const StringViewToCStringHelper pathString(path);

    struct stat stat;
    if (::stat(pathString, &stat) != 0)
    {
        NFE_LOG_ERROR("stat() for '%s' failed: %s", pathString.Str(), strerror(errno));
        return PathType::Invalid;
    }

    if (S_ISREG(stat.st_mode))
        return PathType::File;
    if (S_ISDIR(stat.st_mode))
        return PathType::Directory;

    return PathType::Invalid;
}

bool FileSystem::CreateDir(const StringView& path)
{
    const StringViewToCStringHelper pathString(path);

    if (::mkdir(pathString, 0777) != 0)
    {
        NFE_LOG_ERROR("Failed to create directory '%s': %s", pathString, strerror(errno));
        return false;
    }

    NFE_LOG_INFO("Created directory '%s'", pathString.Str());
    return true;
}

bool FileSystem::Remove(const StringView& path, bool recursive)
{
    const StringViewToCStringHelper pathString(path);

    PathType pathType = GetPathType(path);

    if (pathType == PathType::Directory)
    {
        if (recursive)
        {
            if (!RecursiveDeleteDirectory(path))
            {
                return false;
            }
        }
        else if (::rmdir(pathString) != 0)
        {
            NFE_LOG_ERROR("Failed to remove directory '%s': %s", pathString.Str(), strerror(errno));
            return false;
        }
    }
    else if (pathType == PathType::File)
    {
        if (::unlink(pathString) != 0)
        {
            NFE_LOG_ERROR("Failed to delete file '%s': %s", pathString.Str(), strerror(errno));
            return false;
        }
    }
    else
    {
        return false;
    }

    NFE_LOG_INFO("Removed '%s'", pathString.Str());
    return true;
}

bool FileSystem::Copy(const StringView& srcPath, const StringView& destPath, bool overwrite)
{
    const StringViewToCStringHelper srcPathString(srcPath);
    const StringViewToCStringHelper destPathString(destPath);

    (void)overwrite;

    const int bufferSize = 8192;
    char buf[bufferSize];
    ssize_t nread;

    int fdSrc = ::open(srcPathString, O_RDONLY);
    if (fdSrc < 0)
    {
        NFE_LOG_ERROR("Failed to open file '%s': %s", srcPathString.Str(), strerror(errno));
        return false;
    }

    int fdDest = ::open(destPathString, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fdDest < 0)
    {
        NFE_LOG_ERROR("Failed to open file '%s': %s", destPathString.Str(), strerror(errno));
        goto errorLabel;
    }

    while (nread = ::read(fdSrc, buf, bufferSize), nread > 0)
    {
        char* writePtr = buf;
        do
        {
            ssize_t nwritten = ::write(fdDest, writePtr, nread);

            if (nwritten >= 0)
            {
                nread -= nwritten;
                writePtr += nwritten;
            }
            else if (errno != EINTR)
            {
                NFE_LOG_ERROR("Write to file '%s' failed: %s", destPathString.Str(), strerror(errno));
                goto errorLabel;
            }
        } while (nread > 0);
    }

    // success!
    if (nread == 0)
    {
        ::close(fdDest);
        ::close(fdSrc);
        NFE_LOG_INFO("File '%s' copied to '%s'", srcPathString.Str(), destPathString.Str());
        return true;
    }

    NFE_LOG_ERROR("Read from file '%s' failed: %s", srcPathString.Str(), strerror(errno));

errorLabel:
    ::close(fdSrc);
    if (fdDest != -1)
    {
        ::close(fdDest);
        ::unlink(destPathString.Str());
    }

    return false;
}

bool FileSystem::Move(const StringView& srcPath, const StringView& destPath)
{
    const StringViewToCStringHelper srcPathString(srcPath);
    const StringViewToCStringHelper destPathString(destPath);

    if (::rename(srcPathString,destPathString) != 0)
    {
        NFE_LOG_ERROR("Failed to move file '%s' to '%s': %s", srcPathString.Str(), destPathString.Str(), strerror(errno));
        return false;
    }

    NFE_LOG_INFO("File '%s' moved to '%s'", srcPathString.Str(), destPathString.Str());
    return true;
}

bool FileSystem::Iterate(const StringView& path, const DirIterateCallback& callback)
{
    struct dirent* dir;
    Deque<String> directories;
    String currentDir, foundPath;

    directories.PushBack(String(path));

    while (!directories.Empty())
    {
        currentDir = directories.Back();
        directories.PopBack();

        DIR* d = ::opendir(currentDir.Str());
        if (d == NULL)
        {
            NFE_LOG_ERROR("opendir() failed for path '%s': %s", currentDir.Str(), strerror(errno));
            return false;
        }

        // find all elements in current node directory
        while ((dir = ::readdir(d)) != NULL)
        {
            // ignore special paths
            if (::strcmp(dir->d_name, ".") == 0 || ::strcmp(dir->d_name, "..") == 0)
                continue;

            // accept only directories and regular files
            if (dir->d_type != DT_DIR && dir->d_type != DT_REG)
                continue;

            // ignore multiple '/' or '\'
            // TODO create and use function for path normalization
            while (currentDir.Back() == '/' || currentDir.Back() == '\\')
                currentDir.PopBack();

            foundPath = currentDir + '/' + dir->d_name;
            bool isDir = dir->d_type == DT_DIR;

            if (isDir)
                directories.PushBack(foundPath);

            if (!callback(foundPath, isDir ? PathType::Directory : PathType::File))
            {
                ::closedir(d);
                return true;
            }
        }

        ::closedir(d);
    }

    return true;
}

} // namespace Common
} // namespace NFE
