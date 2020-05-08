#include "PCH.hpp"
#include "BufferInputStream.hpp"
#include "System/Memory.hpp"
#include "Memory/Buffer.hpp"
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

BufferInputStream::BufferInputStream(const Buffer& buffer)
    : BufferInputStream(buffer.Data(), buffer.Size())
{ }

uint64 BufferInputStream::GetSize()
{
    return mSize;
}

bool BufferInputStream::Seek(uint64 position)
{
    if (!mData)
    {
        return false;
    }

    if (position > (uint64)std::numeric_limits<size_t>::max())
    {
        return false;
    }

    if (position < mSize)
    {
        mPos = (size_t)position;
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
