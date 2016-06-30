/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  FileBuffered class definitions.
 */

#include "PCH.hpp"
#include "FileBuffered.hpp"
#include "Logger.hpp"


namespace NFE {
namespace Common {

FileBuffered::FileBuffered()
    : mBufferPosition(0)
    , mBufferOccupied(FILE_BUFFERED_BUFFER_SIZE)
{
}

FileBuffered::FileBuffered(const std::string& path, AccessMode mode, bool overwrite)
    : mFile(path, mode, overwrite)
    , mBufferPosition(0)
    , mBufferOccupied(FILE_BUFFERED_BUFFER_SIZE)
    , mMode(mode)
    , mLastOperation(mode)
{
}

FileBuffered::FileBuffered(FileBuffered&& other)
    : mFile(std::move(other.mFile))
    , mBufferPosition(other.mBufferPosition)
    , mBufferOccupied(other.mBufferOccupied)
    , mMode(other.mMode)
    , mLastOperation(other.mLastOperation)
{
    // Used instead of memmove, because it's faster
    memcpy(mBuffer, other.mBuffer, FILE_BUFFERED_BUFFER_SIZE);
}

FileBuffered::~FileBuffered()
{
}


bool FileBuffered::IsOpened() const
{
    return mFile.IsOpened();
}


bool FileBuffered::Open(const std::string& path, AccessMode access, bool overwrite)
{
    bool result = mFile.Open(path, access, overwrite);
    if (result)
        mLastOperation = access;

    return result;
}


void FileBuffered::Close()
{
    mFile.Close();

    mBufferPosition = 0;
    mBufferOccupied = FILE_BUFFERED_BUFFER_SIZE;
}


size_t FileBuffered::Read(void* data, size_t size)
{
    if ((mMode != AccessMode::Read && mMode != AccessMode::ReadWrite) || !IsOpened())
        return 0;

    if (mLastOperation == AccessMode::Write)
        Flush();

    // Set Reading as last executed operation on file
    mLastOperation = AccessMode::Read;

    // Calculate left space in the buffer
    uint32 leftSpace = FILE_BUFFERED_BUFFER_SIZE - mBufferOccupied;

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
        return mFile.Read(data, size);

    // If user want to read data, that can fit in buffer
    } else
    {
        // If whole buffer is ran through, fill it from file
        if (leftSpace <= 0)
        {
            // We set the start of the buffer and then fill it with data
            mBufferPosition = mFile.GetPos();
            mFile.Read(mBuffer, FILE_BUFFERED_BUFFER_SIZE);

            // Whole buffer is filled with new data
            mBufferOccupied = 0;
            leftSpace = FILE_BUFFERED_BUFFER_SIZE;
        }

        // If user want to read what's available in the buffer, copy it
        if (size <= leftSpace)
        {
            memcpy(data, &mBuffer[mBufferOccupied], size);
            mBufferOccupied += static_cast<uint32>(size);
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

    if (mLastOperation == AccessMode::Read)
        mBufferOccupied = FILE_BUFFERED_BUFFER_SIZE;

    // Set Writing as last executed operation on file
    mLastOperation = AccessMode::Write;

    // Calculate left space in the buffer
    uint32 leftSpace = FILE_BUFFERED_BUFFER_SIZE - mBufferOccupied;

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
            mBufferOccupied += static_cast<uint32>(size);
            return size;

        // Else copy what's available, then read the rest
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
                LOG_ERROR("Invalid seek mode");
                return false;
        }

         // If seek destination is within buffer - no jump's needed, just set the position in buffer
        if (IsWithinBuffer(relativePos))
        {
            mBufferOccupied = static_cast<uint32>(relativePos);
            return true;

        // If seek destination is outside of buffer - set buffer as 'used up' in preparation for jump
        } else
            mBufferOccupied = FILE_BUFFERED_BUFFER_SIZE;

    // If buffer is used for writing - flush it in preparation for jump
    } else if (mLastOperation == AccessMode::Write)
        Flush();

    // Jump
    return mFile.Seek(pos, mode);
}

int64 FileBuffered::GetPos() const
{
    // mBufferPosition may point to the old data when reading, to optimize Seek'n'Read functionality
    // thus position is calculated from File's position
    return mFile.GetPos() - FILE_BUFFERED_BUFFER_SIZE + mBufferOccupied;
}

void FileBuffered::Flush()
{
    if ((mMode != AccessMode::Write && mMode != AccessMode::ReadWrite) || !IsOpened())
        return;

    // If data exists, flush it
    if (mBufferOccupied > 0)
    {
        mFile.Write(mBuffer, mBufferOccupied);
        mBufferPosition = mFile.GetPos();
        mBufferOccupied = 0;
    }
}

bool FileBuffered::IsWithinBuffer(int64 position) const
{
    if (position >= 0 && position <= FILE_BUFFERED_BUFFER_SIZE)
        return true;

    return false;
}

} // namespace Common
} // namespace NFE