/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  FileAsync class declaration.
 */

#pragma once
#include "nfCommon.hpp"
#include "File.hpp"
#include <unordered_set>
#include <functional>
#include <mutex>

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#elif defined(__LINUX__) | defined(__linux__)
#include <signal.h>
#endif // defined(WIN32)

namespace NFE {
namespace Common {

/**
 * Class allowing access to a binary file in a buffered manner.
 */
class NFCOMMON_API FileAsync
{
private:
    using CallbackFunc = std::function<void(void*, FileAsync*, size_t, bool)>; //< Function ptr type for callbacks
    struct AsyncDataStruct;               //< Struct for OS specific data allocation

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

    std::unordered_set<AsyncDataStruct*> mSystemPtrs;
    std::mutex mSetAccessMutex;
    CallbackFunc mCallback;

    bool SafeErasePtr(AsyncDataStruct* ptrToErase);
    bool SafeInsertPtr(AsyncDataStruct* ptrToInsert);

public:
    FileAsync();
    explicit FileAsync(CallbackFunc callback);
    FileAsync(const std::string& path, AccessMode mode, CallbackFunc callback,
              bool overwrite = false);
    FileAsync(FileAsync&& other);
    FileAsync(const FileAsync& other) = delete;
    ~FileAsync();

    /**
     * Check if a file is opened.
     */
    bool IsOpened() const;

    bool wait();

    /**
     * Open a file.
     * For write-only and read-write mode, the file is automatically created if it does not exist.
     * @param path   File path.
     * @param access File access mode.
     * @param overwrite Overwrite a file if it already exists.
     */
    bool Open(const std::string& path, AccessMode access, bool overwrite = false);

    /**
     * Close opened file and cancel all unfinished operations.
     */
    void Close();

    /**
     * Enqueues @p byte sized read from a file.
     * @param data    Target data buffer
     * @param size    Number of bytes to read
     * @param offset  Offset from the beginning of the file
     * @param dataPtr Pointer to the user-defined data structure,
     *                that will be passed to the callback
     * @return True if operation was enqueued successfully.
     *
     * @remarks Enqueued operation will call mReadCallback function, when finished.
     */
    bool Read(void* data, size_t size, uint64 offset, void* dataPtr);

    /**
     * Enqueues @p byte sized write to a file.
     * @param data    Source data buffer
     * @param size    Number of bytes to write
     * @param offset  Offset from the beginning of the file
     * @param dataPtr Pointer to the user-defined data structure,
     *                that will be passed to the callback
     * @return True if operation was enqueued successfully.
     *
     * @remarks Enqueued operation will call mReadCallback function, when finished.
     */
    bool Write(void* data, size_t size, uint64 offset, void* dataPtr);

    /**
     * Get total file size in bytes.
     * @return Non-negative value on success.
     */
    int64 GetSize() const;
};

} // namespace Common
} // namespace NFE
