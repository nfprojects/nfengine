/**
 * @file
 * @author Witek902
 * @brief  StaticBuffer class definition.
 */

#include "PCH.hpp"
#include "StaticBuffer.hpp"
#include "Memory/DefaultAllocator.hpp"
#include "Math/Math.hpp"

namespace NFE {
namespace Common {

StaticBuffer::StaticBuffer()
    : mData(nullptr)
    , mSize(0)
{
}

StaticBuffer::StaticBuffer(const StaticBuffer& other)
    : mData(nullptr)
    , mSize(0)
{
    *this = other;
}

StaticBuffer::StaticBuffer(StaticBuffer&& other)
{
    mSize = other.mSize;
    mData = other.mData;

    other.mData = nullptr;
    other.mSize = 0;
}

StaticBuffer::StaticBuffer(const void* data, const size_t dataSize, const size_t alignment)
    : StaticBuffer()
{
    Set(dataSize, data, alignment);
}

StaticBuffer::~StaticBuffer()
{
    Release();
}

StaticBuffer& StaticBuffer::operator = (const StaticBuffer& other)
{
    if (this != &other)
    {
        // TODO copy alignment?
        Set(other.Size(), other.Data());
    }

    return *this;
}

StaticBuffer& StaticBuffer::operator = (StaticBuffer&& other)
{
    if (this != &other)
    {
        Release();

        mSize = other.mSize;
        mData = other.mData;

        other.mData = nullptr;
        other.mSize = 0;
    }

    return *this;
}

void StaticBuffer::Zero()
{
    if (mSize)
    {
        NFE_ASSERT(mData, "Invalid buffer");
        memset(mData, 0, mSize);
    }
}

bool StaticBuffer::Set(size_t size, const void* newData, const size_t alignment)
{
    NFE_ASSERT(Math::IsPowerOfTwo(alignment), "Invalid alignment: %zu", alignment);

    if (mSize != size)
    {
        Release();

        void* newBuffer = nullptr;
        if (size > 0u)
        {
            newBuffer = NFE_MALLOC(size, alignment);

            if (!newBuffer)
            {
                // memory allocation failed
                return false;
            }
        }

        mData = newBuffer;
        mSize = size;
    }

    if (newData)
    {
        memcpy(mData, newData, size);
    }

    return true;
}

void StaticBuffer::Release()
{
    NFE_FREE(mData);
    mData = nullptr;
    mSize = 0;
}

} // namespace Common
} // namespace NFE
