/**
 * @file   FileSystem.cpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Linux implementation of filesystem utilities.
 */

#include "../stdafx.hpp"
#include "../FileSystem.hpp"
#include "../Logger.hpp"

namespace NFE {
namespace Common {

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
        return PathType::Invalid;

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
    // TODO

    LOG_INFO("Removed '%s'", path.c_str());
    return true;
}

bool FileSystem::Copy(const std::string& srcPath, const std::string& destPath, bool overwrite)
{
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

    // Success!
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
        unlink(destPath.c_str());
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
    // TODO

    return true;
}

} // namespace Common
} // namespace NFE
