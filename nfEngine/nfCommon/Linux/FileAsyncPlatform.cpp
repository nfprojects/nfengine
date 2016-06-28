/**
 * @FileAsync
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Linux implementation of FileAsync class.
 */

#include "../PCH.hpp"
#include "../FileAsync.hpp"
#include "../Logger.hpp"

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
    : FileAsync(nullptr)
{
}

FileAsync::FileAsync(CallbackFuncRef callbackFunc)
    : mFD(INVALID_FD)
    , mMode(AccessMode::No)
    , mCallback(callbackFunc)
{
}

FileAsync::FileAsync(const std::string& path, AccessMode mode, CallbackFuncRef callbackFunc,
                     bool overwrite)
    : mFD(INVALID_FD)
    , mMode(AccessMode::No)
    , mCallback(callbackFunc)
{
    Open(path, mode, overwrite);
}

FileAsync::FileAsync(FileAsync&& other)
{
    Close();

    mFD = other.mFD;
    mMode = other.mMode;
    
    other.mFD = INVALID_FD;
    other.mMode = AccessMode::No;
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
            LOG_ERROR("Invalid file access mode");
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

    if (!IsOpened())
    {
        LOG_ERROR("Failed to open file '%s': %s", path.c_str(), strerror(errno));
        mMode = AccessMode::No;
        return false;
    }

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);

    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sa.sa_sigaction = FinishedOperationsHandler;
    if (sigaction(IO_SIGNAL, &sa, NULL) == -1)
    {
        LOG_ERROR("Failed to set up callback handler: %s", strerror(errno));
        Close();
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
                aio_cancel(mFD, &i->aioData);
                
                // Wait for canelled job to complete
                aio_suspend(&i->aioData, 1, nullptr);
                
                // Release memory
                delete i;
                
            }
            
            // Clear mSystemPtrs of freed pointers
            mSystemPtrs.clear();
        }
        
        // Close file handle
        ::close(mFD);
        mFD = INVALID_FD;
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
    if (!IsOpened() || (mMode != AccessMode::Write && mMode != AccessMode::ReadWrite))
        return false;

    AsyncDataStruct* allocStruct = new AsyncDataStruct();
    allocStruct->isRead = false;
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

void FileAsync::FinishedOperationsHandler(int sig, siginfo_t* si, void* ucontext)
{
    // Unused variables, needed by OS API
    (void)sig;
    (void)ucontext;

    if (si->si_code == SI_ASYNCIO)
    {
        AsyncDataStruct* allocStruct = reinterpret_cast<AsyncDataStruct*>(si->si_value.sival_ptr);
        
        // Check if operation was successfull - if not, then do not call the callback function
        if (0 == aio_error(&allocStruct->aioData))
        {
            FileAsync* instance = allocStruct->instancePtr;
            size_t bytesProcessed = 0;

            bytesProcessed = static_cast<size_t>(aio_return(&allocStruct->aioData));

            if (instance->mCallback)
                instance->mCallback(allocStruct->userData, instance, bytesProcessed, allocStruct->isRead);
        }
        
        instance->SafeErasePtr(allocStruct);
    }
}

} // namespace Common
} // namespace NFE
