/**
 * @file
 * @author Witek902
 * @brief  DynamicBuffer class definition.
 */

#include "PCH.hpp"
#include "DynamicBuffer.hpp"
#include "Memory/DefaultAllocator.hpp"
#include "Math/Math.hpp"

namespace NFE {
namespace Common {

DynamicBuffer::DynamicBuffer()
    : mData(nullptr)
    , mSize(0)
    , mCapacity(0)
    , mAlignment(0)
{
}

DynamicBuffer::DynamicBuffer(const DynamicBuffer& other)
    : mData(nullptr)
    , mSize(0)
    , mCapacity(0)
    , mAlignment(0)
{
    *this = other;
}

DynamicBuffer::DynamicBuffer(DynamicBuffer&& other)
{
    mSize = other.mSize;
    mCapacity = other.mCapacity;
    mData = other.mData;
    mAlignment = other.mAlignment;

    other.mData = nullptr;
    other.mSize = 0;
    other.mCapacity = 0;
    other.mAlignment = 0;
}

DynamicBuffer::DynamicBuffer(const void* data, const size_t dataSize, const size_t alignment)
    : DynamicBuffer()
{
    NFE_ASSERT(Math::IsPowerOfTwo(alignment), "Invalid alignment: %zu", alignment);

    Resize(dataSize, data, alignment);
}

DynamicBuffer::~DynamicBuffer()
{
    Release();
}

DynamicBuffer& DynamicBuffer::operator = (const DynamicBuffer& other)
{
    if (this != &other)
    {
        Resize(other.Size(), other.Data(), other.GetAlignment());
    }

    return *this;
}

DynamicBuffer& DynamicBuffer::operator = (DynamicBuffer&& other)
{
    if (this != &other)
    {
        Release();

        mSize = other.mSize;
        mCapacity = other.mCapacity;
        mData = other.mData;
        mAlignment = other.mAlignment;

        other.mData = nullptr;
        other.mSize = 0;
        other.mCapacity = 0;
        other.mAlignment = 0;
    }

    return *this;
}

void DynamicBuffer::Zero()
{
    if (mSize)
    {
        NFE_ASSERT(mData, "Invalid buffer");
        memset(mData, 0, mSize);
    }
}

bool DynamicBuffer::Resize(size_t size, const void* newData, const size_t alignment)
{
    NFE_ASSERT(Math::IsPowerOfTwo(alignment), "Invalid alignment: %zu", alignment);

    if (!Reserve(size, newData == nullptr, alignment))
    {
        return false;
    }

    if (size > 0 && newData)
    {
        memcpy(mData, newData, size);
        mSize = size;
    }

    mSize = size;
    mAlignment = alignment;

    return true;
}

bool DynamicBuffer::Reserve(size_t size, bool preserveData, const size_t alignment)
{
    NFE_ASSERT(Math::IsPowerOfTwo(alignment), "Invalid alignment: %zu", alignment);

    if (size > mCapacity)
    {
        size_t newCapacity = mCapacity;
        while (size > newCapacity)
        {
            // grow by 50%
            newCapacity += Math::Max<size_t>(1u, newCapacity / 2);
        }

        void* newBuffer = NFE_MALLOC(newCapacity, alignment);
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

void DynamicBuffer::Clear()
{
    mSize = 0;
}

void DynamicBuffer::Release()
{
    NFE_FREE(mData);
    mSize = 0;
    mCapacity = 0;
}

} // namespace Common
} // namespace NFE
