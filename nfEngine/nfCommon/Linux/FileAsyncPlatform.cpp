/**
 * @FileAsync
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Linux implementation of FileAsync class.
 */

#include "../PCH.hpp"
#include "../FileAsync.hpp"
#include "../Logger.hpp"

#include <sys/syscall.h>
#include <unistd.h>

namespace {

#define INVALID_FD       -1
#define NUM_EVENTS       128
#define IOCB_FLAG_RESFD  (1 << 0)

#ifndef __NR_eventfd
#if defined(__x86_64__)
#define __NR_eventfd 284
#elif defined(__i386__)
#define __NR_eventfd 323
#else
#error Cannot detect your OS architecture!
#endif
#endif

NFE_INLINE long io_submit(aio_context_t ctx, long n, struct iocb **paiocb)
{
    return syscall(__NR_io_submit, ctx, n, paiocb);
}

NFE_INLINE long io_cancel(aio_context_t ctx, struct iocb *aiocb, struct io_event *res)
{
    return syscall(__NR_io_cancel, ctx, aiocb, res);
}

NFE_INLINE long io_getevents(aio_context_t ctx, long min_nr, long nr,
             struct io_event *events, struct timespec *tmo)
{
    return syscall(__NR_io_getevents, ctx, min_nr, nr, events, tmo);
}

NFE_INLINE int eventfd(int count)
{
    return syscall(__NR_eventfd, count);
}

} // namespace


namespace NFE {
namespace Common {

// This structure is declared in source file because of platform specific data
struct FileAsync::AsyncDataStruct
{
    ::iocb     ioData;      //< OS-specific data
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
    , mEventFD(INVALID_FD)
    , mMode(AccessMode::No)
    , mCallback(callbackFunc)
{
}

FileAsync::FileAsync(const std::string& path, AccessMode mode, CallbackFuncRef callbackFunc,
                     bool overwrite)
    : mFD(INVALID_FD)
    , mEventFD(INVALID_FD)
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
    mEventFD = other.mEventFD;

    other.mFD = INVALID_FD;
    other.mMode = AccessMode::No;
    other.mEventFD = INVALID_FD;
}

FileAsync::~FileAsync()
{
    Close();
}

bool FileAsync::IsOpened() const
{
    return mFD != INVALID_FD;
}

bool FileAsync::Init()
{
    mEventFD = ::eventfd(0);

    if (!AsyncQueueManager::GetInstance().EnqueueJob(JobDispatcher, mEventFD))
    {
        LOG_ERROR("Failed to add eventFD(%i) to AsyncQueueManager queue: %s", mEventFD, strerror(errno));
        Close();
        return false;
    }
    return true;
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

    mFD = ::open(path.c_str(), flags | O_NONBLOCK, 0644);

    if (!IsOpened())
    {
        LOG_ERROR("Failed to open file '%s': %s", path.c_str(), strerror(errno));
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

        {
            // Block access to mSystemPtrs map before accessing it
            std::lock_guard<std::mutex> guard(mSetAccessMutex);

            // Check if there are any ongoing jobs
            if (!mSystemPtrs.empty())
            {
                aio_context_t queueContext = AsyncQueueManager::GetInstance().GetQueueContext();
                // Cancel all jobs in progress to ensure that no file corruption
                // takes place upon closing file handle
                std::unique_ptr<io_event> ev = std::make_unique<io_event>();
                for (auto i : mSystemPtrs)
                {
                    // Cancel job
                    ::io_cancel(queueContext, &i->ioData, ev.get());

                    delete i;
                }

                // Clear mSystemPtrs of freed pointers
                mSystemPtrs.clear();
            }
        }
        if (mEventFD)
            if (!AsyncQueueManager::GetInstance().DequeueJob(mEventFD))
                LOG_ERROR("Removing eventFD(%i) from AsyncQueueManager failed.", mEventFD);

        // Close file handle
        if (mFD)
            ::close(mFD);
        if (mEventFD)
            ::close(mEventFD);

        mFD = INVALID_FD;
        mEventFD = INVALID_FD;
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

    ::iocb* iocbPtr = &allocStruct->ioData;
    memset(iocbPtr, 0, sizeof(*iocbPtr));
    iocbPtr->aio_fildes = mFD;
    iocbPtr->aio_lio_opcode = IOCB_CMD_PREAD;
    iocbPtr->aio_reqprio = 0;
    iocbPtr->aio_buf = reinterpret_cast<uint64_t>(data);
    iocbPtr->aio_nbytes = size;
    iocbPtr->aio_offset = offset;
    iocbPtr->aio_flags = IOCB_FLAG_RESFD;
    iocbPtr->aio_resfd = mEventFD;
    iocbPtr->aio_data = reinterpret_cast<uint64_t>(allocStruct);

    if (!SafeInsertPtr(allocStruct))
    {
        LOG_ERROR("Error in OS-specific data allocation.");
        delete allocStruct;
        return false;
    }

    ::iocb* cbs[1];
    cbs[0] = iocbPtr;

    int enqueueResult = ::io_submit(AsyncQueueManager::GetInstance().GetQueueContext(), 1, cbs);
    if (enqueueResult < 0)
    {
        LOG_ERROR("FileAsync failed to enqueue read operation[%u]: %s", errno, strerror(errno));
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

    ::iocb* iocbPtr = &allocStruct->ioData;
    memset(iocbPtr, 0, sizeof(*iocbPtr));
    iocbPtr->aio_fildes = mFD;
    iocbPtr->aio_lio_opcode = IOCB_CMD_PWRITE;
    iocbPtr->aio_reqprio = 0;
    iocbPtr->aio_buf = reinterpret_cast<uint64_t>(data);
    iocbPtr->aio_nbytes = size;
    iocbPtr->aio_offset = offset;
    iocbPtr->aio_flags = IOCB_FLAG_RESFD;
    iocbPtr->aio_resfd = mEventFD;
    iocbPtr->aio_data = reinterpret_cast<uint64_t>(allocStruct);

    if (!SafeInsertPtr(allocStruct))
    {
        LOG_ERROR("Error in OS-specific data allocation.");
        delete allocStruct;
        return false;
    }

    ::iocb* cbs[1];
    cbs[0] = iocbPtr;

    int enqueueResult = ::io_submit(AsyncQueueManager::GetInstance().GetQueueContext(), 1, cbs);
    if (enqueueResult < 0)
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

void FileAsync::FinishedOperationsHandler(int64_t result, void* allocStructData)
{
    AsyncDataStruct* allocStruct = reinterpret_cast<AsyncDataStruct*>(allocStructData);
    FileAsync* instance = allocStruct->instancePtr;

    size_t bytesProcessed = static_cast<size_t>(allocStruct->ioData.aio_nbytes);

    // If operation wasn't successfull - log an error and change number of bytes processed to 0
    if (result < 0)
    {
        LOG_ERROR("%s operation for offset:%u and size:%zu failed.",
                  allocStruct->isRead ? "Read" : "Write",
                  allocStruct->ioData.aio_offset,
                  bytesProcessed);

        bytesProcessed = 0;
    }

    if (instance->mCallback)
        instance->mCallback(allocStruct->userData, instance, bytesProcessed, allocStruct->isRead);

    instance->SafeErasePtr(allocStruct);
}

void FileAsync::JobDispatcher(int eventsNo, int eventFD)
{
    ::io_event eventsPollBuffer[NUM_EVENTS];
    int readEvents = 0;
    ::timespec eventsTime;
    eventsTime.tv_sec = 0;
    eventsTime.tv_nsec = 5000; // in nanoseconds

    // Read events
    readEvents = ::io_getevents(AsyncQueueManager::GetInstance().GetQueueContext(), 1, eventsNo, eventsPollBuffer, &eventsTime);

    if (readEvents < 0)
    {
        LOG_ERROR("io_getevents() for FileAsync failed: %s", strerror(errno));
        return;
    } else if (readEvents == 0)
    {
        // This should not happen
        u_int64_t eval = 0;
        LOG_WARNING("io_getevents() for FileAsync returned 0");
        ::read(eventFD, &eval, sizeof(eval));
        return;
    }

    // Process events from the received buffer
    ::io_event* loopLimit = eventsPollBuffer + (readEvents * sizeof(::io_event));
    size_t loopStep = sizeof(::io_event);

    for (::io_event* event = eventsPollBuffer; event < loopLimit; event += loopStep)
    {
        ::iocb* ioPtr = reinterpret_cast<::iocb*>(event->obj);
        if (ioPtr)
            if (ioPtr->aio_resfd == eventFD)
                if (ioPtr->aio_lio_opcode == IOCB_CMD_PWRITE || ioPtr->aio_lio_opcode == IOCB_CMD_PREAD)
                    FileAsync::FinishedOperationsHandler(event->res, reinterpret_cast<void*>(event->data));
    }

}

} // namespace Common
} // namespace NFE
