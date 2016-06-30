/**
 * @FileAsync
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Windows implementation of FileAsync class.
 */

#include "../PCH.hpp"
#include "../FileAsync.hpp"
#include "../Logger.hpp"
#include "Common.hpp"


namespace NFE {
namespace Common {

FileAsync::FileAsync()
    : mFile(INVALID_HANDLE_VALUE)
    , mReadCallback(nullptr)
    , mWriteCallback(nullptr)
{
}

FileAsync::FileAsync(callbackFunc readCallback, callbackFunc writeCallback)
    : mFile(INVALID_HANDLE_VALUE)
    , mReadCallback(readCallback)
    , mWriteCallback(writeCallback)
{
}

FileAsync::FileAsync(const std::string& path, AccessMode mode, callbackFunc readCallback,
                     callbackFunc writeCallback, bool overwrite)
    : mFile(INVALID_HANDLE_VALUE)
    , mReadCallback(readCallback)
    , mWriteCallback(writeCallback)
{
    Open(path, mode, overwrite);
}

FileAsync::FileAsync(FileAsync&& other)
    : mReadCallback(other.mReadCallback)
    , mWriteCallback(other.mWriteCallback)
{
    if (IsOpened())
        ::CloseHandle(mFile);

    mFile = other.mFile;
    other.mFile = INVALID_HANDLE_VALUE;
}

FileAsync::~FileAsync()
{
    if (IsOpened())
        CloseHandle(mFile);
}

bool FileAsync::IsOpened() const
{
    return mFile != INVALID_HANDLE_VALUE;
}

// TODO: access sharing flags
bool FileAsync::Open(const std::string& path, AccessMode access, bool overwrite)
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
            LOG_ERROR("Invalid FileAsync access mode");
            return false;
    }

    DWORD creationDisposition;
    if (access == AccessMode::Read) // when opening for read-only, open only exising FileAsyncs
        creationDisposition = OPEN_EXISTING;
    else
        creationDisposition = overwrite ? CREATE_ALWAYS : OPEN_ALWAYS;

    mFile = ::CreateFile(widePath.c_str(), desiredAccess, FILE_SHARE_READ, NULL,
                         creationDisposition, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);

    if (mFile == INVALID_HANDLE_VALUE)
    {
        LOG_ERROR("Failed to open FileAsync '%s': %s", path.c_str(), GetLastErrorString().c_str());
        return false;
    }

    return true;
}

void FileAsync::Close()
{
    if (!IsOpened())
    {
        CancelIoEx(mFile, nullptr);
        ::CloseHandle(mFile);
        mFile = INVALID_HANDLE_VALUE;
    }
}

bool FileAsync::Read(void* data, size_t size, uint64 offset)
{
    if (!IsOpened())
        return 0;

    LPOVERLAPPED overlapped = new OVERLAPPED();
    overlapped->Offset = offset & 0xFFFFFFFF;
    overlapped->OffsetHigh = (offset >> 32) & 0xFFFFFFFF;
    overlapped->Pointer = 0;


    ioInfoStruct* info = new ioInfoStruct();
    info->isRead = true;
    info->objPtr = this;

    overlapped->hEvent = reinterpret_cast<HANDLE>(info);

    DWORD toRead;
    if (size > static_cast<size_t>(MAXDWORD))
        toRead = MAXDWORD;
    else
        toRead = static_cast<DWORD>(size);

    if (::ReadFileEx(mFile, data, toRead, overlapped,
                reinterpret_cast<LPOVERLAPPED_COMPLETION_ROUTINE>(&FinishedOperationsHandler)) == 0)
    {
        LOG_ERROR("FileAsync read failed: %s", GetLastErrorString().c_str());
        return false;
    }

    return true;
}

bool FileAsync::Write(const void* data, size_t size, uint64 offset)
{
    if (!IsOpened())
        return 0;

    LPOVERLAPPED overlapped = new OVERLAPPED();
    overlapped->Offset = offset & 0xFFFFFFFF;
    overlapped->OffsetHigh = (offset >> 32) & 0xFFFFFFFF;
    overlapped->Pointer = 0;


    ioInfoStruct* info = new ioInfoStruct();
    info->isRead = false;
    info->objPtr = this;

    overlapped->hEvent = reinterpret_cast<HANDLE>(info);

    DWORD toWrite;
    if (size > static_cast<size_t>(MAXDWORD))
        toWrite = MAXDWORD;
    else
        toWrite = static_cast<DWORD>(size);

    if (::WriteFileEx(mFile, data, toWrite, overlapped,
                reinterpret_cast<LPOVERLAPPED_COMPLETION_ROUTINE>(&FinishedOperationsHandler)) == 0)
    {
        LOG_ERROR("FileAsync write failed: %s", GetLastErrorString().c_str());
        return false;
    }

    return true;
}

int64 FileAsync::GetSize() const
{
    if (!IsOpened())
        return -1;

    LARGE_INTEGER size;
    if (::GetFileSizeEx(mFile, &size) == 0)
    {
        LOG_ERROR("GetFileSizeEx failed: %s", GetLastErrorString().c_str());
        return -1;
    }

    return static_cast<int64>(size.QuadPart);
}

bool FileAsync::Seek(int64 pos, SeekMode mode)
{
    // File offset has no effect in Async IO
    (void)mode;
    (void)pos;
    return false;
}

int64 FileAsync::GetPos() const
{
    // There is no 'actual' position in Async IO
    return -1;
}

void FileAsync::FinishedOperationsHandler(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered,
                                          LPOVERLAPPED lpOverlapped)
{
    (void)dwErrorCode;
    (void)dwNumberOfBytesTransfered;

    ioInfoStruct* info;
    info = reinterpret_cast<ioInfoStruct*>(lpOverlapped->hEvent);

    FileAsync* instance = reinterpret_cast<FileAsync*>(info->objPtr);

    if (info->isRead)
    {
         if (instance->mReadCallback)
             (*instance->mReadCallback)();
    }
    else
    {
         if (instance->mWriteCallback)
             (*instance->mWriteCallback)();
    }

    delete lpOverlapped;
    delete info;
}

} // namespace Common
} // namespace NFE
