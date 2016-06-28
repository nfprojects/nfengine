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

namespace {

} // namespace


namespace NFE {
namespace Common {

HANDLE FileAsync::mCallbackThread = ::CreateThread(0, 0, CallbackDispatcher, nullptr, 0, 0);

// This structure is declared in source file because of platform specific data
struct FileAsync::AsyncDataStruct
{
    OVERLAPPED   overlapped;     //< OS-specific data
    DWORD        bytesToProcess; //< Parameter for Read/Write
    bool         isRead;         //< Operation type flag
    FileAsync*   instancePtr;    //< 'this' pointer
    void*        userData;       //< Pointer to user-defined object
    void*        dataBuffer;     //< Parameter for Read/Write
};

FileAsync::FileAsync()
    : FileAsync(nullptr)
{
}

FileAsync::FileAsync(CallbackFuncRef callback)
    : mFile(INVALID_HANDLE_VALUE)
    , mMode(AccessMode::No)
    , mCallback(callback)
{
}

FileAsync::FileAsync(const std::string& path, AccessMode mode, CallbackFuncRef callback,
                     bool overwrite)
    : mFile(INVALID_HANDLE_VALUE)
    , mMode(AccessMode::No)
    , mCallback(callback)
{
    Open(path, mode, overwrite);
}

FileAsync::FileAsync(FileAsync&& other)
    : mCallback(other.mCallback)
{
    Close();

    mFile = other.mFile;
    mMode = other.mMode;

    other.mFile = INVALID_HANDLE_VALUE;
    other.mMode = AccessMode::No;
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
            LOG_ERROR("Invalid file access mode");
            mMode = AccessMode::No;
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
        mMode = AccessMode::No;
        return false;
    }

    return true;
}

void FileAsync::Close()
{
    if (IsOpened())
    {
        // Change file mode to invalid
        mMode = AccessMode::No;

        // Check if there are any ongoing jobs
        if (!mSystemPtrs.empty())
        {
            // Block access to mSystemPtrs map before accessing it
            std::lock_guard<std::mutex> guard(mSetAccessMutex);

            // Cancel all jobs in progress to ensure that no file corruption
            // takes place upon closing file handle
            for (auto i : mSystemPtrs)
            {
                // Cancel job
                CancelIoEx(mFile, &i->overlapped);

                // Wait for cancelled job to complete
                GetOverlappedResult(mFile, &i->overlapped, nullptr, TRUE);

                // Release memory
                delete i;
            }

            // Clear mSystemPtrs of freed pointers
            mSystemPtrs.clear();
        }

        // Close file handle
        ::CloseHandle(mFile);
        mFile = INVALID_HANDLE_VALUE;
    }
}

bool FileAsync::Read(void* data, size_t size, uint64 offset, void* dataPtr)
{
    if (!IsOpened() || (mMode != AccessMode::Read && mMode != AccessMode::ReadWrite))
        return false;

    AsyncDataStruct* allocStruct = new AsyncDataStruct();
    allocStruct->isRead = true;
    allocStruct->instancePtr = this;
    allocStruct->userData = dataPtr;
    allocStruct->dataBuffer = data;

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

    if (size > static_cast<size_t>(MAXDWORD))
        allocStruct->bytesToProcess = MAXDWORD;
    else
        allocStruct->bytesToProcess = static_cast<DWORD>(size);

    // Enqueue ReadFileEx call in our callback thread
    if (0 == ::QueueUserAPC(&ReadProc, mCallbackThread, reinterpret_cast<ULONG_PTR>(allocStruct)))
    {
        LOG_ERROR("QueueUserAPC() failed for read operation: %s", GetLastErrorString().c_str());
        SafeErasePtr(allocStruct);
        return false;
    }

    return true;
}

bool FileAsync::Write(void* data, size_t size, uint64 offset, void* dataPtr)
{
    if (!IsOpened() || (mMode != AccessMode::Write && mMode != AccessMode::ReadWrite))
        return false;

    AsyncDataStruct* allocStruct = new AsyncDataStruct();
    allocStruct->isRead = false;
    allocStruct->instancePtr = this;
    allocStruct->userData = dataPtr;
    allocStruct->dataBuffer = data;

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

    if (size > static_cast<size_t>(MAXDWORD))
        allocStruct->bytesToProcess = MAXDWORD;
    else
        allocStruct->bytesToProcess = static_cast<DWORD>(size);

    // Enqueue WriteFileEx call in our callback thread
    if (0 == ::QueueUserAPC(&WriteProc, mCallbackThread, reinterpret_cast<ULONG_PTR>(allocStruct)))
    {
        LOG_ERROR("QueueUserAPC() failed for write operation: %s", GetLastErrorString().c_str());
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
    AsyncDataStruct* allocStruct = reinterpret_cast<AsyncDataStruct*>(lpOverlapped->hEvent);
    FileAsync* instance = allocStruct->instancePtr;

    // Check if operation was successfull - if not, then do not call the callback function
    if (dwErrorCode == 0)
    {
        size_t bytesProcessed = static_cast<size_t>(dwNumberOfBytesTransfered);

        if (instance->mCallback)
            instance->mCallback(allocStruct->userData, instance, bytesProcessed, allocStruct->isRead);
    }

    instance->SafeErasePtr(allocStruct);
}

DWORD FileAsync::CallbackDispatcher(LPVOID param)
{
    UNUSED(param);

    // Drift in the abyss of idleness waiting for the moment to become useful
    while (true)
        ::SleepEx(INFINITE, true);

    return 0;
}

void FileAsync::ReadProc(ULONG_PTR arg)
{
    using OverlappedCmpRtn = LPOVERLAPPED_COMPLETION_ROUTINE;
    AsyncDataStruct* allocStruct = reinterpret_cast<AsyncDataStruct*>(arg);

    if (0 == ::ReadFileEx(allocStruct->instancePtr->mFile,
                          allocStruct->dataBuffer,
                          allocStruct->bytesToProcess,
                          &allocStruct->overlapped,
                          reinterpret_cast<OverlappedCmpRtn>(&FileAsync::FinishedOperationsHandler)))
    {
        LOG_ERROR("FileAsync failed to enqueue read operation: %s", GetLastErrorString().c_str());
        allocStruct->instancePtr->SafeErasePtr(allocStruct);
    }
}

void FileAsync::WriteProc(ULONG_PTR arg)
{
    using OverlappedCmpRtn = LPOVERLAPPED_COMPLETION_ROUTINE;
    AsyncDataStruct* allocStruct = reinterpret_cast<AsyncDataStruct*>(arg);

    if (0 == ::WriteFileEx(allocStruct->instancePtr->mFile,
                          allocStruct->dataBuffer,
                          allocStruct->bytesToProcess,
                          &allocStruct->overlapped,
                          reinterpret_cast<OverlappedCmpRtn>(&FileAsync::FinishedOperationsHandler)))
    {
        LOG_ERROR("FileAsync failed to enqueue write operation: %s", GetLastErrorString().c_str());
        allocStruct->instancePtr->SafeErasePtr(allocStruct);
    }
}

} // namespace Common
} // namespace NFE
