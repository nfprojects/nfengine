/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  FileAsync class declaration.
 */

#pragma once

#include "../nfCommon.hpp"
#include "File.hpp"
#include "../System/Mutex.hpp"
#include "../Containers/StringView.hpp"
#include "../Containers/HashSet.hpp"

#include <functional>
#include <thread>

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#elif defined(__LINUX__) | defined(__linux__)
#include <unistd.h>     /* for syscall() */
#include <poll.h>
#include <sys/syscall.h>    /* for __NR_* definitions */
#include <linux/aio_abi.h>  /* for AIO types and constants */
#endif // defined(WIN32)

namespace NFE
{
namespace Common
{

/**
 * Class allowing access to a binary file in a buffered manner.
 */
class NFCOMMON_API FileAsync
{
private:
    // Function ptr type for callbacks
    using CallbackFunc = std::function<void(void* userData, FileAsync* filePtr, size_t bytesProcessed, bool isRead)>;
    using CallbackFuncRef = const CallbackFunc&;
    struct AsyncDataStruct; //< Struct for OS specific data allocation

#if defined(WIN32)
    HANDLE mFile;
    static HANDLE mCallbackThread; //< Remote thread used for APC callbacks

    // Handler for finished read/write calls
    static void CALLBACK FinishedOperationsHandler(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered,
                                                   LPOVERLAPPED lpOverlapped);

    // Idle thread that is used purely to call callback functions
    static DWORD CALLBACK CallbackDispatcher(LPVOID param);

    // Simple ReadFileEx caller, so it can be delegated to mCallbackThread
    static void CALLBACK ReadProc(ULONG_PTR arg);

    // Simple WriteFileEx caller, so it can be delegated to mCallbackThread
    static void CALLBACK WriteProc(ULONG_PTR arg);

#elif defined(__LINUX__) | defined(__linux__)
    int mFD;
    static int mEventFD;
    static aio_context_t mCtx;
    static std::thread mCallbackThread;
    static bool mQuitThreadFlag;

    // Handler for finished read/write calls
    static void FinishedOperationsHandler(int64_t result, void* allocStructData);
    static void CallbackDispatcher();
#else
#error "Target system not supported!"
#endif // defined(WIN32)

    AccessMode mMode;
    HashSet<AsyncDataStruct*> mSystemPtrs;
    Mutex mSetAccessMutex;
    CallbackFunc mCallback;

    bool SafeErasePtr(AsyncDataStruct* ptrToErase);
    bool SafeInsertPtr(AsyncDataStruct* ptrToInsert);

public:
    FileAsync();
    explicit FileAsync(CallbackFuncRef callback);
    FileAsync(const StringView path, AccessMode mode, CallbackFuncRef callback, bool overwrite = false);
    FileAsync(FileAsync&& other);
    FileAsync(const FileAsync& other) = delete;
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
    bool Open(const StringView path, AccessMode access, bool overwrite = false);

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

    /**
     * Get mode, that file is opened in.
     */
    AccessMode GetFileMode() const;
};

} // namespace Common
} // namespace NFE
