/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  FileBuffered class declarations.
 */

#pragma once

#include "File.hpp"

namespace NFE {
namespace Common {

#define FILE_BUFFERED_BUFFER_SIZE 2048

class NFCOMMON_API FileBuffered final
{
private:
    File mFile;

    uint64 mBufferPosition;    //< Position of start of the buffer in file
    size_t mBufferOccupied;    //< Position inside of the buffer
    AccessMode mMode;          //< Mode, that file is opened in
    AccessMode mLastOperation; //< Type of last performed operation
    bool mEofReached;          //< Flag indicating if EOF has been reached
    uint8_t mBuffer[FILE_BUFFERED_BUFFER_SIZE];
    size_t mBufferEOFOffset;   //< Offset in buffer resulting from the last read

    bool IsWithinBuffer(int64 position) const;

public:
    FileBuffered();
    FileBuffered(const std::string& path, AccessMode mode, bool overwrite = false);
    FileBuffered(FileBuffered&& other);
    FileBuffered(const FileBuffered& other) = delete;
    ~FileBuffered();

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
    size_t Read(void* data, size_t size);

    /**
     * Write @p size bytes to a file.
     * @param data Source data buffer
     * @param size
     * @return Number of written bytes
     */
    size_t Write(const void* data, size_t size);

    /**
     * Get total file size in bytes.
     * @return Non-negative value on success.
     */
    int64 GetSize();

    /**
     * Change file cursor to a new location.
     * @param pos Cursor offset
     * @param mode Seeking mode
     * @see SeekMode
     */
    bool Seek(int64 pos, SeekMode mode);

    /**
     * Get file cursor position as if the data in the buffer was flushed.
     * @return Non-negative value on success.
     */
    int64 GetPos() const;

    /**
     * Flush buffer to the file.
     *
     * If the last operation was Write, write remaining data from the buffer to the file.
     * If the last operation was Read, move beginning of the buffer to the current position in the buffer.
     */
    void Flush();

    /*
     * Check whether cursor've reached the EOF.
     */
    bool IsAtEOF() const;
};

} // namespace Common
} // namespace NFE