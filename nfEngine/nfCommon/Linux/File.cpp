/**
 * @file   File.cpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Linux implementation of File class.
 */

#include "../stdafx.hpp"
#include "../File.hpp"
#include "../Logger.hpp"

#include <sys/file.h>

namespace NFE {
namespace Common {

#define INVALID_FD -1

File::File()
{
    mFD = INVALID_FD;
}

File::File(const std::string& path, AccessMode mode, bool overwrite)
{
    mFD = INVALID_FD;
    Open(path, mode, overwrite);
}

File::File(File&& other)
{
    if (mFD != INVALID_FD)
        ::close(mFD);

    mFD = other.mFD;
    other.mFD = INVALID_FD;
}

File::~File()
{
    if (mFD != INVALID_FD)
        ::close(mFD);
}

bool File::IsOpened() const
{
    return mFD != INVALID_FD;
}

// TODO: access sharing flags
bool File::Open(const std::string& path, AccessMode access, bool overwrite)
{
    Close();

    int flags;
    switch (access)
    {
        case AccessMode::Read:
            flags = O_RDONLY;
            break;
        case AccessMode::Write:
            flags = O_WRONLY;
            break;
        case AccessMode::ReadWrite:
            flags = O_RDWR;
            break;
        default:
            LOG_ERROR("Invalid file access mode");
            return false;
    }

    flags |= O_CREAT;
    if (overwrite && access != AccessMode::Read)
        flags |= O_TRUNC;

    mFD = ::open(path.c_str(), flags, 0644);

    if (mFD == -1)
    {
        LOG_ERROR("Failed to open file '%s': %s", path.c_str(), strerror(errno));
        return false;
    }

    return true;
}

void File::Close()
{
    if (mFD != INVALID_FD)
    {
        ::close(mFD);
        mFD = INVALID_FD;
    }
}

size_t File::Read(void* data, size_t size)
{
    if (mFD == INVALID_FD)
        return 0;

    ssize_t bytesRead = ::read(mFD, data, size);
    if (bytesRead == -1)
    {
        LOG_ERROR("File read failed: %s", strerror(errno));
        return 0;
    }

    return static_cast<size_t>(bytesRead);
}

size_t File::Write(const void* data, size_t size)
{
    if (mFD == INVALID_FD)
        return 0;

    ssize_t bytesWritten = ::write(mFD, data, size);
    if (bytesWritten == -1)
    {
        LOG_ERROR("File write failed: %s", strerror(errno));
        return 0;
    }

    return static_cast<size_t>(bytesWritten);
}

uint64 File::GetSize() const
{
    if (mFD == INVALID_FD)
        return 0;

    struct stat buf;
    ::fstat(mFD, &buf);
    return static_cast<uint64>(buf.st_size);
}

bool File::Seek(int64 pos, SeekMode mode)
{
    if (mFD == INVALID_FD)
        return false;

    int whence;
    switch (mode)
    {
        case SeekMode::Begin:
            whence = SEEK_SET;
            break;
        case SeekMode::Current:
            whence = SEEK_CUR;
            break;
        case SeekMode::End:
            whence = SEEK_END;
            break;
        default:
            LOG_ERROR("Invalid seek mode");
            return false;
    }

    if (::lseek64(mFD, static_cast<off64_t>(pos), whence) == (off64_t)-1)
    {
        LOG_ERROR("File seek failed: %s", strerror(errno));
        return false;
    }

    return true;
}

uint64 File::GetPos() const
{
    if (mFD == INVALID_FD)
        return false;

    off64_t off = ::lseek64(mFD, 0, SEEK_CUR);
    if (off == (off64_t)-1)
    {
        LOG_ERROR("File seek failed: %s", strerror(errno));
        return 0;
    }

    return static_cast<uint64>(off);
}

} // namespace Common
} // namespace NFE
