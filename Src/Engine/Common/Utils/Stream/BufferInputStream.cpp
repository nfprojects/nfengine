#include "PCH.hpp"
#include "BufferInputStream.hpp"
#include "System/Memory.hpp"
#include "Memory/BufferView.hpp"
#include "Logger/Logger.hpp"


namespace NFE {
namespace Common {

BufferInputStream::BufferInputStream(const void* data, size_t dataSize)
{
    if (!MemoryCheck(data, dataSize))
    {
        NFE_LOG_ERROR("Provided buffer can't be read");
        mData = nullptr;
        mSize = 0;
    }

    mData = data;
    mSize = dataSize;
    mPos = 0;
}

BufferInputStream::BufferInputStream(const BufferView& bufferView)
    : BufferInputStream(bufferView.Data(), bufferView.Size())
{ }

uint64 BufferInputStream::GetSize()
{
    return mSize;
}

bool BufferInputStream::Seek(int64 offset, SeekMode mode)
{
    if (!mData)
    {
        return false;
    }

    uint64 newPosition = std::numeric_limits<uint64>::max();
    switch (mode)
    {
    case SeekMode::Begin:
        newPosition = (uint64)offset;
        break;
    case SeekMode::End:
        newPosition = mSize + offset;
        break;
    case SeekMode::Current:
        newPosition = mPos + offset;
        break;
    default:
        NFE_FATAL("Invalid seek mode");
    }

    if (newPosition < mSize)
    {
        mPos = (size_t)newPosition;
        return true;
    }

    return false;
}

size_t BufferInputStream::Read(void* buffer, size_t num)
{
    if (!mData)
    {
        return 0;
    }

    if (mPos + num > mSize)
    {
        size_t maxReadable = static_cast<size_t>(mSize - mPos);
        memcpy(buffer, reinterpret_cast<const char*>(mData) + mPos, maxReadable);
        mPos = mSize;
        return maxReadable;
    }
    else
    {
        memcpy(buffer, reinterpret_cast<const char*>(mData) + mPos, num);
        mPos += num;
        return num;
    }
}

} // namespace Common
} // namespace NFE
