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
    size_t targetSize = mCursor + numBytes;

    if (!mBuffer.Resize(targetSize))
    {
        return 0;
    }

    memcpy(reinterpret_cast<char*>(mBuffer.Data()) + mCursor, buffer, numBytes);
    mCursor += numBytes;

    return numBytes;
}

bool BufferOutputStream::Seek(uint64 position)
{
    if (position < std::numeric_limits<size_t>::max())
    {
        mCursor = (size_t)position;
        return true;
    }

    return false;
}

} // namespace Common
} // namespace NFE
