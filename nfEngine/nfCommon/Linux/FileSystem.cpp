/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Linux implementation of filesystem utilities.
 */

#include "../PCH.hpp"
#include "../FileSystem.hpp"
#include "../Logger.hpp"

#include <dirent.h>

namespace NFE {
namespace Common {

namespace {

bool RecursiveDeleteDirectory(const std::string& path)
{
    DIR* d;
    struct dirent* dir;
    std::string foundPath;

    d = ::opendir(path.c_str());
    if (d == NULL)
    {
        LOG_ERROR("opendir() failed for path '%s': %s", path.c_str(), strerror(errno));
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
        else if (::unlink(foundPath.c_str()) != 0)
            LOG_ERROR("Failed to delete file '%s': %s", foundPath.c_str(), strerror(errno));
    }

    ::closedir(d);

    // now we can remove empty directory
    if (::rmdir(path.c_str()) != 0)
    {
        LOG_ERROR("Failed to remove directory '%s': %s", path.c_str(), strerror(errno));
        return false;
    }

    return true;
}

} // namespace

std::string FileSystem::GetExecutablePath()
{
    std::string linkPath = "/proc/self/exe";
    std::string execPathStr = "";
    char* execPath = realpath(linkPath.data(), nullptr);

    if (!execPath)
        LOG_ERROR("Failed to resolve executable's path : %s", strerror(errno));
    else
    {
        execPathStr = execPath;
        free(execPath);
    }

    return execPathStr;
}

bool FileSystem::ChangeDirectory(const std::string& path)
{
    if (::chdir(path.c_str()) != 0)
    {
        LOG_ERROR("Failed to change current directory to '%s': %s", path.c_str(), strerror(errno));
        return false;
    }

    LOG_INFO("Current directory changed to: '%s'", path.c_str());
    return true;
}

bool FileSystem::TouchFile(const std::string& path)
{
    int fd = ::open(path.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);

    if (fd < 0)
    {
        LOG_ERROR("Failed to create file '%s': %s", path.c_str(), strerror(errno));
        return false;
    }

    ::close(fd);
    return true;
}

PathType FileSystem::GetPathType(const std::string& path)
{
    struct stat stat;
    if (::stat(path.c_str(), &stat) != 0)
    {
        LOG_ERROR("stat() for '%s' failed: %s", path.c_str(), strerror(errno));
        return PathType::Invalid;
    }

    if (S_ISREG(stat.st_mode))
        return PathType::File;
    if (S_ISDIR(stat.st_mode))
        return PathType::Directory;

    return PathType::Invalid;
}

bool FileSystem::CreateDir(const std::string& path)
{
    if (::mkdir(path.c_str(), 0777) != 0)
    {
        LOG_ERROR("Failed to create directory '%s': %s", path.c_str(), strerror(errno));
        return false;
    }

    LOG_INFO("Created directory '%s'", path.c_str());
    return true;
}

bool FileSystem::Remove(const std::string& path, bool recursive)
{
    PathType pathType = GetPathType(path);

    if (pathType == PathType::Directory)
    {
        if (recursive)
        {
            if (!RecursiveDeleteDirectory(path))
                return false;
        }
        else if (::rmdir(path.c_str()) != 0)
        {
            LOG_ERROR("Failed to remove directory '%s': %s", path.c_str(), strerror(errno));
            return false;
        }
    }
    else if (pathType == PathType::File)
    {
        if (::unlink(path.c_str()) != 0)
        {
            LOG_ERROR("Failed to delete file '%s': %s", path.c_str(), strerror(errno));
            return false;
        }
    }
    else
        return false;

    LOG_INFO("Removed '%s'", path.c_str());
    return true;
}

bool FileSystem::Copy(const std::string& srcPath, const std::string& destPath, bool overwrite)
{
    (void)overwrite;

    const int bufferSize = 8192;
    char buf[bufferSize];
    ssize_t nread;

    int fdSrc = ::open(srcPath.c_str(), O_RDONLY);
    if (fdSrc < 0)
    {
        LOG_ERROR("Failed to open file '%s': %s", srcPath.c_str(), strerror(errno));
        return false;
    }

    int fdDest = ::open(destPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fdDest < 0)
    {
        LOG_ERROR("Failed to open file '%s': %s", destPath.c_str(), strerror(errno));
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
                LOG_ERROR("Write to file '%s' failed: %s", destPath.c_str(), strerror(errno));
                goto errorLabel;
            }
        } while (nread > 0);
    }

    // success!
    if (nread == 0)
    {
        ::close(fdDest);
        ::close(fdSrc);
        LOG_INFO("File '%s' copied to '%s'", srcPath.c_str(), destPath.c_str());
        return true;
    }

    LOG_ERROR("Read from file '%s' failed: %s", srcPath.c_str(), strerror(errno));

errorLabel:
    ::close(fdSrc);
    if (fdDest != -1)
    {
        ::close(fdDest);
        ::unlink(destPath.c_str());
    }

    return false;
}

bool FileSystem::Move(const std::string& srcPath, const std::string& destPath)
{
    if (::rename(srcPath.c_str(), destPath.c_str()) != 0)
    {
        LOG_ERROR("Failed to move file '%s' to '%s': %s", srcPath.c_str(), destPath.c_str(),
                  strerror(errno));
        return false;
    }

    LOG_INFO("File '%s' moved to '%s'", srcPath.c_str(), destPath.c_str());
    return true;
}

bool FileSystem::Iterate(const std::string& path, DirIterateCallback callback)
{
    struct dirent* dir;
    std::stack<std::string> directories;
    std::string currentDir, foundPath;

    directories.push(path);

    while (!directories.empty())
    {
        currentDir = directories.top();
        directories.pop();

        DIR* d = ::opendir(currentDir.c_str());
        if (d == NULL)
        {
            LOG_ERROR("opendir() failed for path '%s': %s", currentDir.c_str(), strerror(errno));
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
            while (currentDir.back() == '/' || currentDir.back() == '\\')
                currentDir.pop_back();

            foundPath = currentDir + '/' + dir->d_name;
            bool isDir = dir->d_type == DT_DIR;

            if (isDir)
                directories.push(foundPath);

            if (!callback(foundPath, isDir))
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
