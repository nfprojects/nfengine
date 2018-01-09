/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Linux implementation of File class.
 */

#include "PCH.hpp"
#include "../File.hpp"
#include "Logger/Logger.hpp"

#include <sys/file.h>

namespace NFE {
namespace Common {

#define INVALID_FD -1

File::File()
    : mFD(INVALID_FD)
    , mMode (AccessMode::No)
{
}

File::File(const std::string& path, AccessMode mode, bool overwrite)
    : mFD(INVALID_FD)
    , mMode (AccessMode::No)
{
    Open(path, mode, overwrite);
}

File::File(File&& other)
{
    Close();

    mFD = other.mFD;
    other.mFD = INVALID_FD;
}

File::~File()
{
    Close();
}

bool File::IsOpened() const
{
    return mFD != INVALID_FD;
}

// TODO: access sharing flags
bool File::Open(const std::string& path, AccessMode access, bool overwrite)
{
    Close();

    mMode = access;
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
            NFE_LOG_ERROR("Invalid file access mode");
            mMode = AccessMode::No;
            return false;
    }

    if (access != AccessMode::Read)
    {
        flags |= O_CREAT;
        if (overwrite)
            flags |= O_TRUNC;
    }

    mFD = ::open(path.c_str(), flags, 0644);

    if (mFD == -1)
    {
        NFE_LOG_ERROR("Failed to open file '%s': %s", path.c_str(), strerror(errno));
        mMode = AccessMode::No;
        return false;
    }

    return true;
}

void File::Close()
{
    if (IsOpened())
    {
        ::close(mFD);
        mFD = INVALID_FD;
        mMode = AccessMode::No;
    }
}

size_t File::Read(void* data, size_t size)
{
    if (!IsOpened() || (mMode != AccessMode::Read && mMode != AccessMode::ReadWrite))
        return 0;

    ssize_t bytesRead = ::read(mFD, data, size);
    if (bytesRead == -1)
    {
        NFE_LOG_ERROR("File read failed: %s", strerror(errno));
        return 0;
    }

    return static_cast<size_t>(bytesRead);
}

size_t File::Write(const void* data, size_t size)
{
    if (!IsOpened() || (mMode != AccessMode::Write && mMode != AccessMode::ReadWrite))
        return 0;

    ssize_t bytesWritten = ::write(mFD, data, size);
    if (bytesWritten == -1)
    {
        NFE_LOG_ERROR("File write failed: %s", strerror(errno));
        return 0;
    }

    return static_cast<size_t>(bytesWritten);
}

int64 File::GetSize() const
{
    if (!IsOpened())
        return -1;

    struct stat buf;
    ::fstat(mFD, &buf);
    return static_cast<int64>(buf.st_size);
}

bool File::Seek(int64 pos, SeekMode mode)
{
    if (!IsOpened())
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
            NFE_LOG_ERROR("Invalid seek mode");
            return false;
    }

    if (::lseek64(mFD, static_cast<off64_t>(pos), whence) == (off64_t)-1)
    {
        NFE_LOG_ERROR("File seek failed: %s", strerror(errno));
        return false;
    }

    return true;
}

int64 File::GetPos() const
{
    if (!IsOpened())
        return -1;

    off64_t off = ::lseek64(mFD, 0, SEEK_CUR);
    if (off == (off64_t)-1)
    {
        NFE_LOG_ERROR("File seek failed: %s", strerror(errno));
        return -1;
    }

    return static_cast<int64>(off);
}

AccessMode File::GetFileMode() const
{
    return mMode;
}

} // namespace Common
} // namespace NFE
