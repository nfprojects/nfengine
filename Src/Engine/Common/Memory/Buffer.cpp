/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Buffer class definition.
 */

#include "PCH.hpp"
#include "Buffer.hpp"
#include "Memory/DefaultAllocator.hpp"
#include "Math/Math.hpp"

namespace NFE {
namespace Common {

Buffer::Buffer()
    : mData(nullptr)
    , mSize(0)
    , mCapacity(0)
{
}

Buffer::Buffer(const Buffer& other)
    : mData(nullptr)
    , mSize(0)
    , mCapacity(0)
{
    *this = other;
}

Buffer::Buffer(Buffer&& other)
{
    mSize = other.mSize;
    mCapacity = other.mCapacity;
    mData = other.mData;

    other.mData = nullptr;
    other.mSize = 0;
    other.mCapacity = 0;
}

Buffer::~Buffer()
{
    Release();
}

Buffer& Buffer::operator = (const Buffer& other)
{
    if (this != &other)
    {
        Resize(other.Size(), other.Data());
    }

    return *this;
}

Buffer& Buffer::operator = (Buffer&& other)
{
    if (this != &other)
    {
        Release();

        mSize = other.mSize;
        mCapacity = other.mCapacity;
        mData = other.mData;

        other.mData = nullptr;
        other.mSize = 0;
        other.mCapacity = 0;
    }

    return *this;
}

void Buffer::Zero()
{
    if (mSize)
    {
        NFE_ASSERT(mData);
        memset(mData, 0, mSize);
    }
}

bool Buffer::Resize(size_t size, const void* newData)
{
    if (!Reserve(size, newData == nullptr))
    {
        return false;
    }

    if (size > 0 && newData)
    {
        memcpy(mData, newData, size);
        mSize = size;
    }

    mSize = size;
    return true;
}

bool Buffer::Reserve(size_t size, bool preserveData)
{
    if (size > mCapacity)
    {
        size_t newCapacity = mCapacity;
        while (size > newCapacity)
        {
            // grow by 50%
            newCapacity += Math::Max<size_t>(1u, newCapacity / 2);
        }

        void* newBuffer = NFE_MALLOC(newCapacity, Alignment);
        if (!newBuffer)
        {
            // memory allocation failed
            return false;
        }

        if (preserveData && mData)
        {
            memcpy(newBuffer, mData, mSize);
        }

        NFE_FREE(mData);
        mData = newBuffer;
        mCapacity = newCapacity;
    }

    return true;
}

void Buffer::Clear()
{
    mSize = 0;
}

void Buffer::Release()
{
    NFE_FREE(mData);
    mSize = 0;
    mCapacity = 0;
}

} // namespace Common
} // namespace NFE
