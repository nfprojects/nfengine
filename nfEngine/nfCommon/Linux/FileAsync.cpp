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

FileAsync::FileAsync()
    : mFD(INVALID_FD)
{
}

FileAsync::FileAsync(callbackFunc readCallback, callbackFunc writeCallback)
    : mFD(INVALID_FD)
    , mReadCallback(readCallback)
    , mWriteCallback(writeCallback)
{
}

FileAsync::FileAsync(const std::string& path, AccessMode mode, callbackFunc readCallback,
                     callbackFunc writeCallback, bool overwrite)
    : mFD(INVALID_FD)
    , mReadCallback(readCallback)
    , mWriteCallback(writeCallback)
{
    Open(path, mode, overwrite);
}

FileAsync::FileAsync(FileAsync&& other)
{
    if (mFD != INVALID_FD)
        ::close(mFD);

    mFD = other.mFD;
    other.mFD = INVALID_FD;
}

FileAsync::~FileAsync()
{
    if (mFD != INVALID_FD)
        ::close(mFD);
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
            LOG_ERROR("Invalid FileAsync access mode");
            return false;
    }

    if (access != AccessMode::Read)
    {
        flags |= O_CREAT;
        if (overwrite)
            flags |= O_TRUNC;
    }

    mFD = ::open(path.c_str(), flags, 0644);

    if (mFD == -1)
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
    if (mFD != INVALID_FD)
    {
        aio_cancel(mFD, nullptr);
        ::close(mFD);
        mFD = INVALID_FD;
    }
}

bool FileAsync::Read(void* data, size_t size, uint64 offset)
{
    if (mFD == INVALID_FD)
        return 0;

    ioInfoStruct* info = new ioInfoStruct();
    aiocb* aiocbPtr = new aiocb();
    
    info->isRead = true;
    info->objPtr = this;
    info->infoPtr = aiocbPtr;
    
    aiocbPtr->aio_fildes = mFD;
    aiocbPtr->aio_offset = offset;
    aiocbPtr->aio_buf = data;
    aiocbPtr->aio_nbytes = size;
    aiocbPtr->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    aiocbPtr->aio_sigevent.sigev_signo = IO_SIGNAL;
    aiocbPtr->aio_sigevent.sigev_value.sival_ptr = info;

    int enqueueResult = aio_read(aiocbPtr);
    if (enqueueResult == -1)
    {
        LOG_ERROR("FileAsync failed to enqueue read operation: %s", strerror(errno));
        return false;
    }

    return true;
}

bool FileAsync::Write(void* data, size_t size, uint64 offset)
{
    // If file is opened with O_APPEND, offset is not used - all is written at the EOF
    // but who cares...we don't use it anyway...
    if (mFD == INVALID_FD)
        return 0;

    ioInfoStruct* info = new ioInfoStruct();
    aiocb* aiocbPtr = new aiocb();
    
    info->isRead = false;
    info->objPtr = this;
    info->infoPtr = aiocbPtr;
    
    aiocbPtr->aio_fildes = mFD;
    aiocbPtr->aio_offset = offset;
    aiocbPtr->aio_buf = data;
    aiocbPtr->aio_nbytes = size;
    aiocbPtr->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    aiocbPtr->aio_sigevent.sigev_signo = IO_SIGNAL;
    aiocbPtr->aio_sigevent.sigev_value.sival_ptr = info;

    int enqueueResult = aio_write(aiocbPtr);
    if (enqueueResult == -1)
    {
        LOG_ERROR("FileAsync failed to enqueue write operation: %s", strerror(errno));
        return false;
    }

    return true;
}

int64 FileAsync::GetSize() const
{
    if (mFD == INVALID_FD)
        return -1;

    struct stat buf;
    ::fstat(mFD, &buf);
    return static_cast<int64>(buf.st_size);
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

void FileAsync::FinishedOperationsHandler(int sig, siginfo_t* si, void* ucontext)
{
    (void)sig;
    (void)ucontext;
        
    if (si->si_code == SI_ASYNCIO)
    {
        ioInfoStruct* info = reinterpret_cast<ioInfoStruct*>(si->si_value.sival_ptr);
        FileAsync* instance = reinterpret_cast<FileAsync*>(info->objPtr);

        if (info->isRead)
        {
            if (instance->mReadCallback)
                (*instance->mReadCallback)();
        } else
        {
            if(instance->mWriteCallback)
                (*instance->mWriteCallback)();
        }

        delete reinterpret_cast<aiocb*>(info->infoPtr);
        delete info;
    }
}

} // namespace Common
} // namespace NFE
