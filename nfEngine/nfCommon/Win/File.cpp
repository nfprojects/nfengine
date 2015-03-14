/**
 * @file   File.cpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Windows implementation of File class.
 */

#include "../stdafx.hpp"
#include "../File.hpp"
#include "../Logger.hpp"
#include "Common.hpp"

namespace NFE {
namespace Common {

File::File()
{
    mFile = INVALID_HANDLE_VALUE;
}

File::File(const std::string& path, AccessMode mode, bool overwrite)
{
    mFile = INVALID_HANDLE_VALUE;
    Open(path, mode, overwrite);
}

File::File(File&& other)
{
    if (IsOpened())
        ::CloseHandle(mFile);

    mFile = other.mFile;
    other.mFile = INVALID_HANDLE_VALUE;
}

File::~File()
{
    if (IsOpened())
        CloseHandle(mFile);
}

bool File::IsOpened() const
{
    return mFile != INVALID_HANDLE_VALUE;
}

// TODO: access sharing flags
bool File::Open(const std::string& path, AccessMode access, bool overwrite)
{
    Close();

    std::wstring widePath;
    if (!UTF8ToUTF16(path, widePath))
    {
        mFile = INVALID_HANDLE_VALUE;
        return false;
    }

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
            LOG_ERROR("Invalid file access mode");
            return false;
    }

    DWORD creationDisposition = overwrite ? CREATE_ALWAYS : OPEN_ALWAYS;
    mFile = ::CreateFile(widePath.c_str(), desiredAccess, FILE_SHARE_READ, NULL,
                         creationDisposition, FILE_ATTRIBUTE_NORMAL, 0);

    if (mFile == INVALID_HANDLE_VALUE)
    {
        LOG_ERROR("Failed to open file '%s': %s", path.c_str(), GetLastErrorString().c_str());
        return false;
    }

    return true;
}

void File::Close()
{
    if (!IsOpened())
    {
        ::CloseHandle(mFile);
        mFile = INVALID_HANDLE_VALUE;
    }
}

size_t File::Read(void* data, size_t size)
{
    if (!IsOpened())
        return 0;

    DWORD toRead;
    if (size > static_cast<size_t>(MAXDWORD))
        toRead = MAXDWORD;
    else
        toRead = static_cast<DWORD>(size);

    DWORD read = 0;
    if (::ReadFile(mFile, data, toRead, &read, 0) == 0)
        LOG_ERROR("File read failed: %s", GetLastErrorString().c_str());

    return static_cast<size_t>(read);
}

size_t File::Write(const void* data, size_t size)
{
    if (!IsOpened())
        return 0;

    DWORD toWrite;
    if (size > static_cast<size_t>(MAXDWORD))
        toWrite = MAXDWORD;
    else
        toWrite = static_cast<DWORD>(size);

    DWORD written = 0;
    if (::WriteFile(mFile, data, toWrite, &written, 0) == 0)
        LOG_ERROR("File write failed: %s", GetLastErrorString().c_str());

    return static_cast<size_t>(written);
}

uint64 File::GetSize() const
{
    if (!IsOpened())
        return 0;

    LARGE_INTEGER size;
    if (::GetFileSizeEx(mFile, &size) == 0)
    {
        LOG_ERROR("GetFileSizeEx failed: %s", GetLastErrorString().c_str());
        return false;
    }

    return static_cast<uint64>(size.QuadPart);
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
            LOG_ERROR("Invalid seek mode");
            return false;
    }

    LARGE_INTEGER posLarge;
    posLarge.QuadPart = static_cast<LONGLONG>(pos);
    if (::SetFilePointerEx(mFile, posLarge, NULL, moveMethod) == 0)
    {
        LOG_ERROR("File seek failed: %s", GetLastErrorString().c_str());
        return false;
    }

    return true;
}

uint64 File::GetPos() const
{
    if (!IsOpened())
        return 0;

    LARGE_INTEGER posLarge, pos;
    posLarge.QuadPart = 0;
    if (::SetFilePointerEx(mFile, posLarge, &pos, FILE_CURRENT) == 0)
    {
        LOG_ERROR("File seek failed: %s", GetLastErrorString().c_str());
        return 0;
    }

    return static_cast<uint64>(pos.QuadPart);
}

} // namespace Common
} // namespace NFE
