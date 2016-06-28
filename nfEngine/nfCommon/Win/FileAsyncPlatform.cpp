/**
 * @FileAsync
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Windows implementation of FileAsync class.
 */

#include "../PCH.hpp"
#include "../FileAsync.hpp"
#include "../Logger.hpp"
#include "Common.hpp"
#include "../nfCommon.hpp"


namespace NFE {
namespace Common {

// This structure is declared in source file because of platform specific data
struct FileAsync::AsyncDataStruct
{
    OVERLAPPED   overlapped;  //< OS-specific data
    bool         isRead;      //< Operation type flag
    FileAsync*   instancePtr; //< 'this' pointer
    void*        userData;    //< Pointer to user-defined object
};

FileAsync::FileAsync()
    : FileAsync(nullptr)
{
}

FileAsync::FileAsync(CallbackFuncRef callback)
    : mFile(INVALID_HANDLE_VALUE)
    , mCallback(callback)
{
}

FileAsync::FileAsync(const std::string& path, AccessMode mode, CallbackFuncRef callback,
                     bool overwrite)
    : mFile(INVALID_HANDLE_VALUE)
    , mCallback(callback)
{
    Open(path, mode, overwrite);
}

FileAsync::FileAsync(FileAsync&& other)
    : mCallback(other.mCallback)
{
    if (IsOpened())
        Close();

    mFile = other.mFile;
    other.mFile = INVALID_HANDLE_VALUE;
}

FileAsync::~FileAsync()
{
    Close();
}

bool FileAsync::IsOpened() const
{
    return mFile != INVALID_HANDLE_VALUE;
}

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
            LOG_ERROR("Invalid file access mode");
            return false;
    }

    DWORD creationDisposition;
    if (access == AccessMode::Read) // when opening for read-only, open only exising FileAsyncs
        creationDisposition = OPEN_EXISTING;
    else
        creationDisposition = overwrite ? CREATE_ALWAYS : OPEN_ALWAYS;

    mFile = ::CreateFile(widePath.c_str(), desiredAccess, FILE_SHARE_READ, NULL,
                         creationDisposition, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);

    if(!IsOpened())
    {
        LOG_ERROR("Failed to open file '%s': %s", path.c_str(), GetLastErrorString().c_str());
        return false;
    }

    return true;
}

void FileAsync::Close()
{
    if (IsOpened())
    {
        // Cancel all jobs in progress to ensure that no file corruption
        // takes place upon closing file handle
        //CancelIoEx(mFile, nullptr);

        // Close file handle
        ::CloseHandle(mFile);
        mFile = INVALID_HANDLE_VALUE;

        // Deallocate all data used by cancelled jobs
        for (auto i : mSystemPtrs)
            delete i;

        // then clear the set
        mSystemPtrs.clear();
    }
}

bool FileAsync::Read(void* data, size_t size, uint64 offset, void* dataPtr)
{
    if (!IsOpened())
        return 0;

    AsyncDataStruct* allocStruct = new AsyncDataStruct();
    allocStruct->isRead = true;
    allocStruct->instancePtr = this;
    allocStruct->userData = dataPtr;

    LPOVERLAPPED overlappedPtr = &allocStruct->overlapped;
    overlappedPtr->Offset = offset & 0xFFFFFFFF;
    overlappedPtr->OffsetHigh = offset >> 32;
    overlappedPtr->hEvent = reinterpret_cast<HANDLE>(allocStruct);

    if (!SafeInsertPtr(allocStruct))
    {
        LOG_ERROR("Error in OS-specific data allocation.");
        delete allocStruct;
        return false;
    }

    DWORD toRead;
    if (size > static_cast<size_t>(MAXDWORD))
        toRead = MAXDWORD;
    else
        toRead = static_cast<DWORD>(size);

    if (::ReadFileEx(mFile, data, toRead, overlappedPtr,
                reinterpret_cast<LPOVERLAPPED_COMPLETION_ROUTINE>(&FinishedOperationsHandler)) == 0)
    {
        LOG_ERROR("FileAsync failed to enqueue read operation: %s", GetLastErrorString().c_str());
        SafeErasePtr(allocStruct);
        return false;
    }

    return true;
}

// TODO: Remove
bool FileAsync::Wait()
{
    DWORD bytesRead;
    for (auto i : mSystemPtrs)
    {
        GetOverlappedResult(mFile, &i->overlapped, &bytesRead, TRUE);
        LOG_DEBUG("Read %u bytes for offset %u\n", static_cast<size_t>(bytesRead), i->overlapped.Offset);
    }
    return true;
}

bool FileAsync::Write(void* data, size_t size, uint64 offset, void* dataPtr)
{
    if (!IsOpened())
        return 0;

    AsyncDataStruct* allocStruct = new AsyncDataStruct();
    allocStruct->isRead = false;
    allocStruct->instancePtr = this;
    allocStruct->userData = dataPtr;

    LPOVERLAPPED overlappedPtr = &allocStruct->overlapped;
    overlappedPtr->Offset = offset & 0xFFFFFFFF;
    overlappedPtr->OffsetHigh = (offset >> 32) & 0xFFFFFFFF;
    overlappedPtr->hEvent = reinterpret_cast<HANDLE>(allocStruct);

    if (!SafeInsertPtr(allocStruct))
    {
        LOG_ERROR("Error in OS-specific data allocation.");
        delete allocStruct;
        return false;
    }

    DWORD toWrite;
    if (size > static_cast<size_t>(MAXDWORD))
        toWrite = MAXDWORD;
    else
        toWrite = static_cast<DWORD>(size);

    if (::WriteFileEx(mFile, data, toWrite, overlappedPtr,
                reinterpret_cast<LPOVERLAPPED_COMPLETION_ROUTINE>(&FinishedOperationsHandler)) == 0)
    {
        LOG_ERROR("FileAsync failed to enqueue write operation: %s", GetLastErrorString().c_str());
        SafeErasePtr(allocStruct);
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

void FileAsync::FinishedOperationsHandler(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered,
                                          LPOVERLAPPED lpOverlapped)
{
    UNUSED(dwErrorCode);

    size_t bytesProcessed = static_cast<size_t>(dwNumberOfBytesTransfered);

    AsyncDataStruct* allocStruct = reinterpret_cast<AsyncDataStruct*>(lpOverlapped->hEvent);
    FileAsync* instance = allocStruct->instancePtr;
    if (instance->mCallback)
        instance->mCallback(allocStruct->userData, instance, bytesProcessed, allocStruct->isRead);

    instance->SafeErasePtr(allocStruct);
}

} // namespace Common
} // namespace NFE
