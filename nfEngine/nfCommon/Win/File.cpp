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

using namespace Windows;

File::File()
{
    mFile = INVALID_HANDLE_VALUE;
}

File::File(const std::string& path, AccessMode mode)
{
    mFile = INVALID_HANDLE_VALUE;
    Open(path, mode);
}

File::File(File&& other)
{
    if (mFile != INVALID_HANDLE_VALUE)
        ::CloseHandle(mFile);

    mFile = other.mFile;
    other.mFile = NULL;
}

File::~File()
{
    if (mFile != INVALID_HANDLE_VALUE)
        CloseHandle(mFile);
}

bool File::Open(const std::string& path, AccessMode access, AccessMode share)
{
    Close();

    std::wstring widePath;
    if (!UTF8ToUTF16(path, widePath))
    {
        mFile = NULL;
        return false;
    }

    DWORD desiredAccess;
    switch (access)
    {
    case AccessMode::None:
        desiredAccess = 0;
        break;
    case AccessMode::Read:
        desiredAccess = GENERIC_READ;
        break;
    case AccessMode::Write:
        desiredAccess = GENERIC_WRITE;
        break;
    case AccessMode::ReadWrite:
        desiredAccess = GENERIC_READ|GENERIC_WRITE;
        break;
    default:
        LOG_ERROR("Invalid file access mode");
        return false;
    }

    DWORD shareMode;
    switch (share)
    {
    case AccessMode::None:
        shareMode = 0;
        break;
    case AccessMode::Read:
        shareMode = FILE_SHARE_READ;
        break;
    case AccessMode::Write:
        shareMode = FILE_SHARE_WRITE;
        break;
    case AccessMode::ReadWrite:
        shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
        break;
    default:
        LOG_ERROR("Invalid file share mode");
        return false;
    }

    mFile = ::CreateFile(widePath.c_str(), desiredAccess, shareMode, NULL, CREATE_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL, 0);

    if (mFile == INVALID_HANDLE_VALUE)
    {
        LOG_ERROR("Failed to open file '%s': %s", path.c_str(), GetLastErrorString().c_str());
        return false;
    }

    return true;
}

void File::Close()
{
    if (mFile != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(mFile);
        mFile = INVALID_HANDLE_VALUE;
    }
}

size_t File::Read(void* data, size_t size)
{
    if (mFile == INVALID_HANDLE_VALUE)
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
    if (mFile == INVALID_HANDLE_VALUE)
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
    if (mFile == INVALID_HANDLE_VALUE)
        return 0;

    DWORD low, high;
    low = ::GetFileSize(mFile, &high);
    return static_cast<uint64>(low) | (static_cast<uint64>(high) << 32);
}

bool File::Seek(int64 pos, SeekMode mode)
{
    if (mFile == INVALID_HANDLE_VALUE)
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

    LONG posHigh = static_cast<LONG>(pos >> 32);
    LONG posLow = static_cast<LONG>(pos & 0xFFFFFFFFL);
    if (::SetFilePointer(mFile, posLow, &posHigh, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        LOG_ERROR("File seek failed: %s", GetLastErrorString().c_str());
        return false;
    }

    return true;
}

uint64 File::GetPos() const
{
    // TODO
    return 0;
}

} // namespace Common
} // namespace NFE
