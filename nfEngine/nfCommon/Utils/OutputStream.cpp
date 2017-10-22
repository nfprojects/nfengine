/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkkulagowski (mkulagowski(at)gmail.com)
 * @brief  Definition of OutputStream class for writing files, buffers, etc.
 */

#include "PCH.hpp"
#include "OutputStream.hpp"
#include "Memory/DefaultAllocator.hpp"


namespace NFE {
namespace Common {

// ===============================================================
// FileOutputStream
// ===============================================================

FileOutputStream::FileOutputStream(const String& fileName)
{
    mFile.Open(fileName, AccessMode::Write, true);
}

size_t FileOutputStream::Write(const void* buffer, size_t num)
{
    return mFile.Write(buffer, num);
}


// ===============================================================
// BufferOutputStream
// ===============================================================

BufferOutputStream::BufferOutputStream()
{
    mBufferSize = 0;
    mUsed = 0;
    mData = nullptr;
}

BufferOutputStream::~BufferOutputStream()
{
    Clear();
}

void BufferOutputStream::Clear()
{
    if (mData)
    {
        NFE_FREE(mData);
        mData = 0;
    }

    mBufferSize = 0;
    mUsed = 0;
}

const void* BufferOutputStream::GetData() const
{
    return mData;
}

size_t BufferOutputStream::GetSize() const
{
    return mUsed;
}

size_t BufferOutputStream::Write(const void* buffer, size_t num)
{
    if (!mData)
    {
        mBufferSize = 1;
        while (mBufferSize < num)
            mBufferSize *= 2;

        mData = NFE_MALLOC(mBufferSize, 1);
        if (!mData)
            return 0;

        mUsed = 0;
    }
    else
    {
        size_t minSize = mUsed + num;
        if (mBufferSize < minSize)
        {
            size_t oldSize = mBufferSize;
            while (mBufferSize < minSize)
                mBufferSize *= 2;

            void* newData = NFE_MALLOC(mBufferSize, 1);
            if (newData)
                memcpy(newData, mData, oldSize);
            NFE_FREE(mData);
            mData = newData;

            if (!mData)
                return 0;
        }
    }

    memcpy(reinterpret_cast<char*>(mData) + mUsed, buffer, num);
    mUsed += num;
    return num;
}

} // namespace Common
} // namespace NFE
