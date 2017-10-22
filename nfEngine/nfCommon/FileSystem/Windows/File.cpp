/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Windows implementation of File class.
 */

#include "PCH.hpp"
#include "../File.hpp"
#include "Logger/Logger.hpp"
#include "System/Win/Common.hpp"


namespace NFE {
namespace Common {

File::File()
    : mFile(INVALID_HANDLE_VALUE)
    , mMode (AccessMode::No)
{
}

File::File(const StringView path, AccessMode mode, bool overwrite)
    : mFile(INVALID_HANDLE_VALUE)
    , mMode(AccessMode::No)
{
    Open(path, mode, overwrite);
}

File::File(File&& other)
{
    Close();

    mFile = other.mFile;
    other.mFile = INVALID_HANDLE_VALUE;
}

File::~File()
{
    Close();
}

bool File::IsOpened() const
{
    return mFile != INVALID_HANDLE_VALUE;
}

// TODO: access sharing flags
bool File::Open(const StringView path, AccessMode access, bool overwrite)
{
    Close();

    Utf16String widePath;
    if (!UTF8ToUTF16(path, widePath))
    {
        mFile = INVALID_HANDLE_VALUE;
        mMode = AccessMode::No;
        return false;
    }
    mMode = access;
    DWORD desiredAccess;
    switch (access)
    {
        case AccessMode::Read:
            desiredAccess = GENERIC_READ;
            break;
        case AccessMode::Write:
            desiredAccess = GENERIC_WRITE;
            break;
        case AccessMode::ReadWrite:
            desiredAccess = GENERIC_READ | GENERIC_WRITE;
            break;
        default:
            NFE_LOG_ERROR("Invalid file access mode");
            mMode = AccessMode::No;
            return false;
    }

    DWORD creationDisposition;
    if (access == AccessMode::Read) // when opening for read-only, open only exising files
        creationDisposition = OPEN_EXISTING;
    else
        creationDisposition = overwrite ? CREATE_ALWAYS : OPEN_ALWAYS;

    mFile = ::CreateFile(widePath.Data(), desiredAccess, FILE_SHARE_READ, NULL,
                         creationDisposition, FILE_ATTRIBUTE_NORMAL, 0);

    if (mFile == INVALID_HANDLE_VALUE)
    {
        NFE_LOG_ERROR("Failed to open file '%.*s': %s", path.Length(), path.Data(), GetLastErrorString().Str());
        mMode = AccessMode::No;
        return false;
    }

    return true;
}

void File::Close()
{
    if (IsOpened())
    {
        ::CloseHandle(mFile);
        mFile = INVALID_HANDLE_VALUE;
        mMode = AccessMode::No;
    }
}

size_t File::Read(void* data, size_t size)
{
    if (!IsOpened() || (mMode != AccessMode::Read && mMode != AccessMode::ReadWrite))
        return 0;

    DWORD toRead;
    if (size > static_cast<size_t>(MAXDWORD))
        toRead = MAXDWORD;
    else
        toRead = static_cast<DWORD>(size);

    DWORD read = 0;
    if (::ReadFile(mFile, data, toRead, &read, 0) == 0)
        NFE_LOG_ERROR("File read failed: %s", GetLastErrorString().Str());

    return static_cast<size_t>(read);
}

size_t File::Write(const void* data, size_t size)
{
    if (!IsOpened() || (mMode != AccessMode::Write && mMode != AccessMode::ReadWrite))
        return 0;

    DWORD toWrite;
    if (size > static_cast<size_t>(MAXDWORD))
        toWrite = MAXDWORD;
    else
        toWrite = static_cast<DWORD>(size);

    DWORD written = 0;
    if (::WriteFile(mFile, data, toWrite, &written, 0) == 0)
        NFE_LOG_ERROR("File write failed: %s", GetLastErrorString().Str());

    return static_cast<size_t>(written);
}

int64 File::GetSize() const
{
    if (!IsOpened())
        return -1;

    LARGE_INTEGER size;
    if (::GetFileSizeEx(mFile, &size) == 0)
    {
        NFE_LOG_ERROR("GetFileSizeEx failed: %s", GetLastErrorString().Str());
        return -1;
    }

    return static_cast<int64>(size.QuadPart);
}

bool File::Seek(int64 pos, SeekMode mode)
{
    if (!IsOpened())
        return false;

    DWORD moveMethod;
    switch (mode)
    {
        case SeekMode::Begin:
            moveMethod = FILE_BEGIN;
            break;
        case SeekMode::Current:
            moveMethod = FILE_CURRENT;
            break;
        case SeekMode::End:
            moveMethod = FILE_END;
            break;
        default:
            NFE_LOG_ERROR("Invalid seek mode");
            return false;
    }

    LARGE_INTEGER posLarge;
    posLarge.QuadPart = static_cast<LONGLONG>(pos);
    if (::SetFilePointerEx(mFile, posLarge, NULL, moveMethod) == 0)
    {
        NFE_LOG_ERROR("File seek failed: %s", GetLastErrorString().Str());
        return false;
    }

    return true;
}

int64 File::GetPos() const
{
    if (!IsOpened())
        return -1;

    LARGE_INTEGER posLarge, pos;
    posLarge.QuadPart = 0;
    if (::SetFilePointerEx(mFile, posLarge, &pos, FILE_CURRENT) == 0)
    {
        NFE_LOG_ERROR("File seek failed: %s", GetLastErrorString().Str());
        return -1;
    }

    return static_cast<int64>(pos.QuadPart);
}

AccessMode File::GetFileMode() const
{
    return mMode;
}

} // namespace Common
} // namespace NFE
