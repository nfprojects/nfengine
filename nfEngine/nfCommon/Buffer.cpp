/**
    NFEngine project

    \file   Buffer.cpp
    \brief  Buffer class definition.
*/

#include "stdafx.h"
#include "Buffer.h"

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
        mData = realloc(mData, mSize);
        memcpy(mData, src.mData, mSize);
    }
    else
    {
        mData = nullptr;
    }
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
    mSize = src.mSize;
    if (mSize)
    {
        mData = realloc(mData, mSize);
        memcpy(mData, src.mData, mSize);
    }

    return *this;
}

void Buffer::Create(size_t size)
{
    mSize = size;
    mData = malloc(size);
}

void Buffer::Load(void* pData, size_t size)
{
    mSize = size;
    mData = realloc(mData, size);
    memcpy(mData, pData, size);
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
