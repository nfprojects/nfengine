/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Buffer class definition.
 */

#include "PCH.hpp"
#include "Buffer.hpp"
#include "Memory/DefaultAllocator.hpp"

#include <stdlib.h>
#include <string.h>

namespace NFE {
namespace Common {

Buffer::Buffer()
{
    mData = nullptr;
    mSize = 0;
}

Buffer::Buffer(const Buffer& src)
{
    *this = src;
}

Buffer::Buffer(Buffer&& other)
{
    mSize = other.mSize;
    mData = other.mData;
    other.mData = nullptr;
    other.mSize = 0;
}

Buffer::~Buffer()
{
    Release();
}

Buffer& Buffer::operator=(const Buffer& src)
{
    if (this != &src)
    {
        Release();

        if (src.mSize)
        {
            mData = NFE_MALLOC(src.mSize, 1);
            if (!mData)
                return *this;
            memcpy(mData, src.mData, src.mSize);
        }

        mSize = src.mSize;
    }

    return *this;
}

void Buffer::Create(size_t size)
{
    mSize = size;
    mData = NFE_MALLOC(size, 1);
}

void Buffer::Load(const void* data, size_t size)
{
    Release();

    if (size > 0)
    {
        mData = NFE_MALLOC(size, 1);
        if (!mData)
            return;

        memcpy(mData, data, size);
        mSize = size;
    }
}

void Buffer::Release()
{
    mSize = 0;
    if (nullptr != mData)
    {
        NFE_FREE(mData);
        mData = nullptr;
    }
}

size_t Buffer::GetSize() const
{
    return mSize;
}

void* Buffer::GetData() const
{
    return mData;
}

} // namespace Common
} // namespace NFE
