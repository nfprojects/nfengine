/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  FileBuffered class definitions.
 */

#include "PCH.hpp"
#include "FileBuffered.hpp"
#include "Logger/Logger.hpp"

#include <string.h>


namespace NFE {
namespace Common {

FileBuffered::FileBuffered()
    : mBufferPosition(0)
    , mBufferOccupied(FILE_BUFFERED_BUFFER_SIZE)
    , mLastOperation(AccessMode::No)
    , mEofReached(false)
    , mBufferEOFOffset(0)
{
}

FileBuffered::FileBuffered(const StringView path, AccessMode mode, bool overwrite)
    : mFile(path, mode, overwrite)
    , mBufferPosition(0)
    , mBufferOccupied(FILE_BUFFERED_BUFFER_SIZE)
    , mMode(mode)
    , mLastOperation(AccessMode::No)
    , mEofReached(false)
    , mBufferEOFOffset(0)
{
}

FileBuffered::FileBuffered(FileBuffered&& other)
    : mFile(std::move(other.mFile))
    , mBufferPosition(other.mBufferPosition)
    , mBufferOccupied(other.mBufferOccupied)
    , mMode(other.mMode)
    , mLastOperation(other.mLastOperation)
    , mEofReached(other.mEofReached)
    , mBufferEOFOffset(other.mBufferEOFOffset)
{
    // Used instead of memmove, because it's faster
    memcpy(mBuffer, other.mBuffer, FILE_BUFFERED_BUFFER_SIZE);
    memset(other.mBuffer, 0, FILE_BUFFERED_BUFFER_SIZE);
}

FileBuffered::~FileBuffered()
{
    Close();
}

bool FileBuffered::IsOpened() const
{
    return mFile.IsOpened();
}

bool FileBuffered::Open(const StringView path, AccessMode access, bool overwrite)
{
    bool result = mFile.Open(path, access, overwrite);
    if (result)
        mMode = access;

    mEofReached = false;
    mBufferEOFOffset = 0;

    return result;
}

void FileBuffered::Close()
{
    Flush();
    mFile.Close();

    mBufferPosition = 0;
    mMode = AccessMode::No;
    mBufferOccupied = FILE_BUFFERED_BUFFER_SIZE;
    mEofReached = false;
    mBufferEOFOffset = 0;
}

size_t FileBuffered::Read(void* data, size_t size)
{
    if ((mMode != AccessMode::Read && mMode != AccessMode::ReadWrite) || !IsOpened() || IsAtEOF())
        return 0;

    if (mLastOperation == AccessMode::Write)
        Flush();

    // Set Reading as last executed operation on file
    mLastOperation = AccessMode::Read;

    // Calculate left space in the buffer
    size_t leftSpace = FILE_BUFFERED_BUFFER_SIZE - mBufferOccupied;

    // If user want to read more then the buffer is capable of
    if (size > FILE_BUFFERED_BUFFER_SIZE)
    {
        // If buffer is read midway through, read it & then read the rest
        if (leftSpace > 0)
        {
            memcpy(data, &mBuffer[mBufferOccupied], leftSpace);
            mBufferOccupied = FILE_BUFFERED_BUFFER_SIZE;

            return leftSpace + Read(reinterpret_cast<char*>(data) + leftSpace, size - leftSpace);
        }

        // If whole buffer is ran through, read straight from the file
        // We don't change buffer position - it still contains old data
        size_t readBytes = mFile.Read(data, size);

        if (readBytes < size)
        {
            mEofReached = true;
            mBufferEOFOffset = 0;
        }

        return readBytes;

    // If user want to read data, that can fit in buffer
    } else
    {
        // If whole buffer is ran through, fill it from file
        if (leftSpace <= 0)
        {
            // We set the start of the buffer and then fill it with data
            mBufferPosition = mFile.GetPos();
            leftSpace = mFile.Read(mBuffer, FILE_BUFFERED_BUFFER_SIZE);
            mBufferOccupied = 0;

            // Check if EOF is within range
            if (leftSpace < FILE_BUFFERED_BUFFER_SIZE)
            {
                // Check what space is left after filling the buffer
                mBufferEOFOffset = FILE_BUFFERED_BUFFER_SIZE - leftSpace;

                // Move data accordingly and move buffer position counter
                memmove(mBuffer + mBufferEOFOffset, mBuffer, leftSpace);
                mBufferOccupied = mBufferEOFOffset;

                // Set EOF flag
                mEofReached = true;
            }
        }

        // If user want to read what's available in the buffer, copy it
        if (size <= leftSpace)
        {
            memcpy(data, &mBuffer[mBufferOccupied], size);
            mBufferOccupied += size;
            return size;

        // Else copy what's available, then read the rest
        } else
        {
            memcpy(data, &mBuffer[mBufferOccupied], leftSpace);
            mBufferOccupied += leftSpace;
            return leftSpace + Read(reinterpret_cast<char*>(data) + leftSpace, size - leftSpace);
        }
    }
}

size_t FileBuffered::Write(const void* data, size_t size)
{
    if ((mMode != AccessMode::Write && mMode != AccessMode::ReadWrite) || !IsOpened())
        return 0;

    if (mLastOperation != AccessMode::Write)
    {
        if (mLastOperation == AccessMode::Read)
            Flush();
        mBufferOccupied = 0;
    }

    // Set Writing as last executed operation on file
    mLastOperation = AccessMode::Write;

    // Calculate left space in the buffer
    size_t leftSpace = FILE_BUFFERED_BUFFER_SIZE - mBufferOccupied;

    // If user want to write more then the buffer is capable of
    if (size > FILE_BUFFERED_BUFFER_SIZE)
    {
        // If buffer is written midway through, we have to flush remaining data first
        if (leftSpace > 0)
            Flush();

        // We write data straight to the file
        // Buffer is positioned always at the end of currently written data
        auto bytesWritten = mFile.Write(data, size);
        mBufferPosition = mFile.GetPos();
        return bytesWritten;

    // If user want to write data, that can fit in buffer
    } else
    {
        // If buffer is full, flush it and write the data
        if (leftSpace <= 0)
        {
            Flush();
            leftSpace = FILE_BUFFERED_BUFFER_SIZE;
        }


        // If user want to write data, that can fit into remaining buffer space, copy it
        if (size <= leftSpace)
        {
            memcpy(&mBuffer[mBufferOccupied], data, size);
            mBufferOccupied += size;
            return size;

        // Else copy what's available, then write the rest
        } else
        {
            memcpy(&mBuffer[mBufferOccupied], data, leftSpace);
            mBufferOccupied += leftSpace;

            return leftSpace + Write(reinterpret_cast<const char*>(data) + leftSpace,
                                     size - leftSpace);
        }
    }
}

int64 FileBuffered::GetSize()
{
    // If last operation was writing, flush data to the file first
    if (mLastOperation == AccessMode::Write)
        Flush();

    return mFile.GetSize();
}

bool FileBuffered::Seek(int64 pos, SeekMode mode)
{
    if (mLastOperation == AccessMode::Read)
    {
        // It's crucial to find, if seek destination is located within buffer
        int64 relativePos = 0;
        switch (mode)
        {
            case SeekMode::Begin:
                relativePos = pos - mBufferPosition;
                break;
            case SeekMode::Current:
                relativePos = pos;
                break;
            case SeekMode::End:
                relativePos = pos + GetSize();
                break;
            default:
                NFE_LOG_ERROR("Invalid seek mode");
                return false;
        }

         // If seek destination is within buffer - no jump's needed, just set the position in buffer
        if (IsWithinBuffer(relativePos))
        {
            mBufferOccupied = static_cast<size_t>(relativePos) + mBufferEOFOffset;
            return true;
        // If seek destination is outside of buffer - set buffer as 'used up' in preparation for jump
        }
        else
        {
            mBufferOccupied = FILE_BUFFERED_BUFFER_SIZE;
            // Reset EOF flag and offset
            mEofReached = false;
            mBufferEOFOffset = 0;
        }

    // If buffer is used for writing - flush it in preparation for jump
    } else if (mLastOperation == AccessMode::Write)
        Flush();

    // Jump
    return mFile.Seek(pos, mode);
}

int64 FileBuffered::GetPos() const
{
    if (!IsOpened())
        return -1;

    if (mLastOperation == AccessMode::Write)
        return mBufferPosition + mBufferOccupied;

    // When EOF is reached, there's an offset in the buffer, otherwise it's equal 0
    return mBufferPosition + mBufferOccupied - mBufferEOFOffset;
}

void FileBuffered::Flush()
{
    if (!IsOpened())
        return;

    if (mLastOperation == AccessMode::Read)
    {
        mFile.Seek(GetPos(), SeekMode::Begin);
        mBufferOccupied = FILE_BUFFERED_BUFFER_SIZE;
    }
    else if (mLastOperation == AccessMode::Write && mBufferOccupied > 0)
    {
        mFile.Write(mBuffer, mBufferOccupied);
        mBufferPosition = mFile.GetPos();
        mBufferOccupied = 0;
    }
}

bool FileBuffered::IsWithinBuffer(int64 position) const
{
    int64 buffEnd = FILE_BUFFERED_BUFFER_SIZE;
    if (mEofReached)
        buffEnd -= mBufferEOFOffset;

    return (position >= 0 && position < buffEnd);
}

NFE_INLINE bool FileBuffered::IsAtEOF() const
{
    return (mEofReached && mBufferOccupied == FILE_BUFFERED_BUFFER_SIZE);
}

} // namespace Common
} // namespace NFE