/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  FileAsync class declaration.
 */

#pragma once
#include "nfCommon.hpp"
#include "File.hpp"

#if defined(WIN32)
#include <Windows.h>
#endif

namespace NFE {
namespace Common {

/**
 * Class allowing access to a binary file in a buffered manner.
 */
class NFCOMMON_API FileAsync
{
private:
    using callbackFunc = std::function<void()>*;

#if defined(WIN32)
    HANDLE mFile;

    static void FinishedOperationsHandler(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered,
                                   LPOVERLAPPED lpOverlapped);
#elif defined(__LINUX__) | defined(__linux__)
    int mFD;

    static void FinishedOperationsHandler(int sig, siginfo_t *si, void *ucontext);
#else
#error "Target system not supported!"
#endif
    callbackFunc mReadCallback;
    callbackFunc mWriteCallback;

public:
    struct ioInfoStruct
    {
        bool isRead;
        void* infoPtr;
        void* objPtr;
    };

    FileAsync();
    FileAsync(callbackFunc readCallback, callbackFunc writeCallback);
    FileAsync(const std::string& path, AccessMode mode, callbackFunc readCallback,
              callbackFunc writeCallback, bool overwrite = false);
    FileAsync(FileAsync&& other);
    ~FileAsync();

    /**
     * Check if a file is opened.
     */
    bool IsOpened() const;

    /**
     * Open a file.
     * For write-only and read-write mode, the file is automatically created if it does not exist.
     * @param path   File path.
     * @param access File access mode.
     * @param overwrite Overwrite a file if it already exists.
     */
    bool Open(const std::string& path, AccessMode access, bool overwrite = false);

    /**
     * Close opened file.
     */
    void Close();

    /**
     * Reads @p size bytes from a file.
     * @param data Target data buffer
     * @param size
     * @return Number of read bytes
     */
    bool Read(void* data, size_t size, uint64 offset);

    /**
     * Write @p size bytes to a file.
     * @param data Source data buffer
     * @param size
     * @return Number of written bytes
     */
    bool Write(const void* data, size_t size, uint64 offset);

    /**
     * Get total file size in bytes.
     * @return Non-negative value on success.
     */
    int64 GetSize() const;

    /**
     * Change file cursor to a new location.
     * @param pos Cursor offset
     * @param mode Seeking mode
     * @see SeekMode
     */
    bool Seek(int64 pos, SeekMode mode);

    /**
     * Get current file cursor position.
     * @return Non-negative value on success.
     */
    int64 GetPos() const;
};

} // namespace Common
} // namespace NFE
