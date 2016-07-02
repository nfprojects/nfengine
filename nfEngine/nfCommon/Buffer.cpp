/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Buffer class definition.
 */

#include "PCH.hpp"
#include "Buffer.hpp"

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
    mSize = src.mSize;
    if (mSize)
    {
        void* newData = realloc(mData, mSize);
        if (newData == nullptr)
        {
            Release();
            return;
        }

        mData = newData;
        memcpy(mData, src.mData, mSize);
    }
    else
        Release();
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
        mSize = src.mSize;
        if (mSize)
        {
            void* newData = realloc(mData, mSize);
            if (newData)
            {
                mData = newData;
                memcpy(mData, src.mData, mSize);
            }
            else
                Release();
        }
    }

    return *this;
}

void Buffer::Create(size_t size)
{
    mSize = size;
    mData = malloc(size);
}

void Buffer::Load(const void* pData, size_t size)
{
    mSize = size;
    void* newData = realloc(mData, size);

    if (newData)
    {
        mData = newData;
        memcpy(mData, pData, size);
    }
    else
        Release();
}

void Buffer::Release()
{
    mSize = 0;
    if (nullptr != mData)
    {
        free(mData);
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
