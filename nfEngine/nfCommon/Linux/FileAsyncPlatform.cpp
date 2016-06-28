/**
 * @FileAsync
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Linux implementation of FileAsync class.
 */

#include "../PCH.hpp"
#include "../FileAsync.hpp"
#include "../Logger.hpp"


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

NFE_INLINE long io_setup(unsigned nr_reqs, aio_context_t *ctx)
{
    return syscall(__NR_io_setup, nr_reqs, ctx);
}
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

bool setupIo(::aio_context_t& ctx)
{
    auto res = ::io_setup(NUM_EVENTS, &ctx);
    if (res < 0)
    {
        LOG_ERROR("FileAsync failed to setup aio_context[%u]: %s", errno, strerror(errno));
        return false;
    }
    return true;
}

} // namespace


namespace NFE {
namespace Common {

int FileAsync::mEventFD = ::eventfd(0);
::aio_context_t FileAsync::mCtx = 0;
std::thread FileAsync::mCallbackThread;
bool FileAsync::mQuitThreadFlag = false;

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
    , mMode(AccessMode::No)
    , mCallback(callbackFunc)
{

    if(!mCallbackThread.joinable())
    {
        mQuitThreadFlag = false;
        mCallbackThread = std::thread(&FileAsync::CallbackDispatcher);
    }

    if (!mEventFD)
        mEventFD = ::eventfd(0);

    if (!mCtx)
        setupIo(mCtx);
}

FileAsync::FileAsync(const std::string& path, AccessMode mode, CallbackFuncRef callbackFunc,
                     bool overwrite)
    : mFD(INVALID_FD)
    , mMode(AccessMode::No)
    , mCallback(callbackFunc)
{
    if(!mCallbackThread.joinable())
    {
        mQuitThreadFlag = false;
        mCallbackThread = std::thread(&FileAsync::CallbackDispatcher);
    }

    if (!mEventFD)
        mEventFD = ::eventfd(0);

    if (!mCtx)
        setupIo(mCtx);

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
    mQuitThreadFlag = true;
    mCallbackThread.join();
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
                // Cancel all jobs in progress to ensure that no file corruption
                // takes place upon closing file handle
                std::unique_ptr<io_event> ev = std::make_unique<io_event>();
                for (auto i : mSystemPtrs)
                {
                    // Cancel job
                    ::io_cancel(mCtx, &i->ioData, ev.get());

                    delete i;
                }

                // Clear mSystemPtrs of freed pointers
                mSystemPtrs.clear();
            }
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
    int enqueueResult = ::io_submit(mCtx, 1, cbs);
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
    int enqueueResult = ::io_submit(mCtx, 1, cbs);
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

void FileAsync::CallbackDispatcher()
{
    const int pollTimeout = 500; // in milliseconds
    ::io_event eventsPollBuffer[NUM_EVENTS];
    ::pollfd pollDescriptor;
    int waitingEvents = 0, readEvents = 0;
    ::timespec eventsTime;
    eventsTime.tv_sec = 0;
    eventsTime.tv_nsec = pollTimeout * 1000; // in nanoseconds

    while (!FileAsync::mQuitThreadFlag)
    {
        pollDescriptor = {mEventFD, POLLIN, 0};
        waitingEvents = ::poll(&pollDescriptor, 1, pollTimeout);
        if (waitingEvents < 0)
        {
            LOG_ERROR("poll() for FileAsync failed: %s", strerror(errno));
            break;
        }
        else if (waitingEvents == 0) // Timeout
            continue;

        // Read events
        if (mCtx)
            readEvents = ::io_getevents(mCtx, 1, waitingEvents, eventsPollBuffer, &eventsTime);
        else
            break;

        if (readEvents < 0)
        {
            if (mCtx)
                LOG_ERROR("io_getevents() for FileAsync failed: %s", strerror(errno));
            break;
        }
        else if (readEvents == 0)
        {
            // This should not happen
            u_int64_t eval = 0;
            LOG_WARNING("io_getevents() for FileAsync returned 0");
            ::read(mEventFD, &eval, sizeof(eval));
            continue;
        }

        // Process events from the received buffer
        ::io_event* loopLimit = eventsPollBuffer + (readEvents * sizeof(::io_event));
        size_t loopStep = sizeof(::io_event);

        for (::io_event* event = eventsPollBuffer; event < loopLimit; event += loopStep)
        {
            ::iocb* ioPtr = reinterpret_cast<::iocb*>(event->obj);
            if (ioPtr)
                if (ioPtr->aio_lio_opcode == IOCB_CMD_PWRITE || ioPtr->aio_lio_opcode == IOCB_CMD_PREAD)
                    FileAsync::FinishedOperationsHandler(event->res, reinterpret_cast<void*>(event->data));
        }
    }
}

} // namespace Common
} // namespace NFE
