#include "PCH.hpp"
#include "BufferOutputStream.hpp"
#include "Memory/Buffer.hpp"


namespace NFE {
namespace Common {

BufferOutputStream::BufferOutputStream(Buffer& targetBuffer)
    : mBuffer(targetBuffer)
{
    mCursor = mBuffer.Size();
}

BufferOutputStream::~BufferOutputStream()
{
}

const void* BufferOutputStream::GetData() const
{
    return mBuffer.Data();
}

size_t BufferOutputStream::GetSize() const
{
    return mBuffer.Size();
}

size_t BufferOutputStream::Write(const void* buffer, size_t numBytes)
{
    NFE_ASSERT(mCursor <= mBuffer.Size(), "Cursor (%zu) is past the buffer end (%zu)", mCursor, mBuffer.Size());

    const size_t targetSize = mCursor + numBytes;

    if (mBuffer.Size() < targetSize)
    {
        if (!mBuffer.Resize(targetSize))
        {
            return 0;
        }
    }

    memcpy(reinterpret_cast<char*>(mBuffer.Data()) + mCursor, buffer, numBytes);
    mCursor += numBytes;

    return numBytes;
}

uint64 BufferOutputStream::GetPosition() const
{
    return mCursor;
}

bool BufferOutputStream::Seek(int64 offset, SeekMode mode)
{
    uint64 newPosition = std::numeric_limits<uint64>::max();
    switch (mode)
    {
    case SeekMode::Begin:
        newPosition = (uint64)offset;
        break;
    case SeekMode::End:
        newPosition = mBuffer.Size() + offset;
        break;
    case SeekMode::Current:
        newPosition = mCursor + offset;
        break;
    default:
        NFE_FATAL("Invalid seek mode");
    }

    if (newPosition <= mBuffer.Size())
    {
        mCursor = (size_t)newPosition;
        return true;
    }

    return false;
}

} // namespace Common
} // namespace NFE
