/**
 * @FileAsync
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Linux implementation of FileAsync class.
 */

#include "../PCH.hpp"
#include "../FileAsync.hpp"
#include "../Logger.hpp"

#include <aio.h>

#define IO_SIGNAL SIGUSR1
#define INVALID_FD -1

namespace NFE {
namespace Common {

// This structure is declared in source file because of platform specific data
struct FileAsync::AsyncDataStruct
{
    aiocb      aioData;     //< OS-specific data
	bool       isRead;      //< Operation type flag
	FileAsync* instancePtr; //< 'this' pointer
	void*      userData;    //< Pointer to user-defined object
};

FileAsync::FileAsync()
    : mFD(INVALID_FD)
{
}

FileAsync::FileAsync(CallbackFunc readCallback, CallbackFunc writeCallback)
    : mFD(INVALID_FD)
    , mReadCallback(readCallback)
    , mWriteCallback(writeCallback)
{
}

FileAsync::FileAsync(const std::string& path, AccessMode mode, CallbackFunc readCallback,
                     CallbackFunc writeCallback, bool overwrite)
    : mFD(INVALID_FD)
    , mReadCallback(readCallback)
    , mWriteCallback(writeCallback)
{
    Open(path, mode, overwrite);
}

FileAsync::FileAsync(FileAsync&& other)
{
    if (mFD != INVALID_FD)
        Close();

    mFD = other.mFD;
    other.mFD = INVALID_FD;
}

FileAsync::~FileAsync()
{
    Close();
}

bool FileAsync::IsOpened() const
{
    return mFD != INVALID_FD;
}

bool FileAsync::Open(const std::string& path, AccessMode access, bool overwrite)
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

    if (access != AccessMode::Read)
    {
        flags |= O_CREAT;
        if (overwrite)
            flags |= O_TRUNC;
    }

    mFD = ::open(path.c_str(), flags, 0644);

    if (!IsOpened())
    {
        LOG_ERROR("Failed to open file '%s': %s", path.c_str(), strerror(errno));
        return false;
    }

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);

    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sa.sa_sigaction = FinishedOperationsHandler;
    if (sigaction(IO_SIGNAL, &sa, NULL) == -1)
    {
        LOG_ERROR("Failed to set up callback handler: %s", strerror(errno));
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
        aio_cancel(mFD, nullptr);

        // Close file handle
        ::close(mFD);
        mFD = INVALID_FD;

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

    aiocb* aiocbPtr = &allocStruct->aioData;
    aiocbPtr->aio_fildes = mFD;
    aiocbPtr->aio_offset = offset;
    aiocbPtr->aio_buf = data;
    aiocbPtr->aio_nbytes = size;
    aiocbPtr->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    aiocbPtr->aio_sigevent.sigev_signo = IO_SIGNAL;
    aiocbPtr->aio_sigevent.sigev_value.sival_ptr = allocStruct;

    if (!SafeInsertPtr(allocStruct))
    {
        LOG_ERROR("Error in OS-specific data allocation.");
        delete allocStruct;
        return false;
    }

    int enqueueResult = aio_read(aiocbPtr);
    if (enqueueResult == -1)
    {
        LOG_ERROR("FileAsync failed to enqueue read operation: %s", strerror(errno));
        SafeErasePtr(allocStruct);
        return false;
    }

    return true;
}

bool FileAsync::Write(void* data, size_t size, uint64 offset, void* dataPtr)
{
    // If file is opened with O_APPEND, offset is not used - all is written at the EOF
    // but who cares...we don't use it anyway...
    if (!IsOpened())
        return 0;

    AsyncDataStruct* allocStruct = new AsyncDataStruct();
    allocStruct->isRead = true;
    allocStruct->instancePtr = this;
    allocStruct->userData = dataPtr;

    aiocb* aiocbPtr = &allocStruct->aioData;
    aiocbPtr->aio_fildes = mFD;
    aiocbPtr->aio_offset = offset;
    aiocbPtr->aio_buf = data;
    aiocbPtr->aio_nbytes = size;
    aiocbPtr->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    aiocbPtr->aio_sigevent.sigev_signo = IO_SIGNAL;
    aiocbPtr->aio_sigevent.sigev_value.sival_ptr = allocStruct;

    if (!SafeInsertPtr(allocStruct))
    {
        LOG_ERROR("Error in OS-specific data allocation.");
        delete allocStruct;
        return false;
    }

    int enqueueResult = aio_write(aiocbPtr);
    if (enqueueResult == -1)
    {
        LOG_ERROR("FileAsync failed to enqueue write operation: %s", strerror(errno));
        SafeErasePtr(allocStruct);
        return false;
    }

    return true;
}

int64 FileAsync::GetSize() const
{
    if (!IsOpened())
        return -1;

    struct stat buf;
    ::fstat(mFD, &buf);
    return static_cast<int64>(buf.st_size);
}

bool FileAsync::SafeErasePtr(AsyncDataStruct* ptrToErase)
{
    std::lock_guard<std::mutex> guard(mSetAccessMutex);
    delete ptrToErase;
    return 1 == mSystemPtrs.erase(ptrToErase);
}

bool FileAsync::SafeInsertPtr(AsyncDataStruct* ptrToInsert)
{
    std::lock_guard<std::mutex> guard(mSetAccessMutex);
    return mSystemPtrs.insert(ptrToInsert).second;
}

void FileAsync::FinishedOperationsHandler(int sig, siginfo_t* si, void* ucontext)
{
    // Unused variables, needed by OS API
    (void)sig;
    (void)ucontext;

    if (si->si_code == SI_ASYNCIO)
    {
        AsyncDataStruct* allocStruct = reinterpret_cast<AsyncDataStruct*>(si->si_value.sival_ptr);
        FileAsync* instance = allocStruct->instancePtr;

        if (allocStruct->isRead)
        {
            if (instance->mReadCallback)
                (*instance->mReadCallback)(allocStruct->userData);
        } else
        {
            if(instance->mWriteCallback)
                (*instance->mWriteCallback)(allocStruct->userData);
        }

    	instance->SafeErasePtr(allocStruct);
    }
}

} // namespace Common
} // namespace NFE
