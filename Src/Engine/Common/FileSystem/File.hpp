/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  File class declaration.
 */

#pragma once

#include "../nfCommon.hpp"

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace NFE {
namespace Common {

/**
 * File seeking mode.
 */
enum class SeekMode
{
    Begin,   ///< relative to file's beginning
    Current, ///< relative to current cursor position
    End,     ///< relative to file's end
};

/**
 * File access mode.
 */
enum class AccessMode
{
    No,
    Read,
    Write,
    ReadWrite,
};

/**
 * Class allowing access to a binary file.
 */
class NFCOMMON_API File
{
private:
#if defined(WIN32)
    HANDLE mFile;
#elif defined(__LINUX__) | defined(__linux__)
    int mFD;
#else
#error "Target system not supported!"
#endif
    AccessMode mMode;

public:
    File();
    File(const String& path, AccessMode mode, bool overwrite = false);
    File(File&& other);
    ~File();

    /**
     * Check if a file is opened.
     */
    bool virtual IsOpened() const;

    /**
     * Open a file.
     * For write-only and read-write mode, the file is automatically created if it does not exist.
     * @param path   File path.
     * @param access File access mode.
     * @param overwrite Overwrite a file if it already exists.
     */
    bool virtual Open(const String& path, AccessMode access, bool overwrite = false);

    /**
     * Close opened file.
     */
    void virtual Close();

    /**
     * Reads @p size bytes from a file.
     * @param data Target data buffer
     * @param size
     * @return Number of read bytes
     */
    size_t virtual Read(void* data, size_t size);

    /**
     * Write @p size bytes to a file.
     * @param data Source data buffer
     * @param size
     * @return Number of written bytes
     */
    size_t virtual Write(const void* data, size_t size);

    /**
     * Get total file size in bytes.
     * @return Non-negative value on success.
     */
    int64 virtual GetSize() const;

    /**
     * Change file cursor to a new location.
     * @param pos Cursor offset
     * @param mode Seeking mode
     * @see SeekMode
     */
    bool virtual Seek(int64 pos, SeekMode mode);

    /**
     * Get current file cursor position.
     * @return Non-negative value on success.
     */
    int64 virtual GetPos() const;

    /**
     * Get mode, that file is opened in.
     */
    AccessMode virtual GetFileMode() const;
};

} // namespace Common
} // namespace NFE
