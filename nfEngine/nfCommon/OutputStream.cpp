/**
 * @file   OutputStream.cpp
 * @author Witek902 (witek902@gmail.com)
 * @author mkkulagowski (mkulagowski@users.noreply.github.com)
 * @brief  Definition of OutputStream class for writing files, buffers, etc.
 */

#include "stdafx.hpp"
#include "OutputStream.hpp"

namespace NFE {
namespace Common {

OutputStream::~OutputStream()
{
}


// ===============================================================
// FileOutputStream
// ===============================================================

FileOutputStream::FileOutputStream(const char* pFileName)
{
    mFile.Open(pFileName, AccessMode::Write, true);
}

FileOutputStream::~FileOutputStream()
{
    mFile.Close();
}

// Write 'num' bytes read from pSrc. Returns number of written bytes.
size_t FileOutputStream::Write(const void* pSrc, size_t num)
{
    return mFile.Write(pSrc, num);
}


// ===============================================================
// BufferOutputStream
// ===============================================================

BufferOutputStream::BufferOutputStream()
{
    mBufferSize = 0;
    mUsed = 0;
    mData = 0;
}

BufferOutputStream::~BufferOutputStream()
{
    Clear();
}

void BufferOutputStream::Clear()
{
    if (mData)
    {
        free(mData);
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

// Write 'num' bytes read from pSrc. Returns number of written bytes.
size_t BufferOutputStream::Write(const void* pSrc, size_t num)
{
    if (!mData)
    {
        mBufferSize = 1;
        while (mBufferSize < num)
            mBufferSize *= 2;

        mData = malloc(mBufferSize);
        if (!mData)
            return 0;

        mUsed = 0;
    }
    else
    {
        size_t minSize = mUsed + num;
        if (mBufferSize < minSize)
        {
            while (mBufferSize < minSize)
                mBufferSize *= 2;

            void* newData = realloc(mData, mBufferSize);
            if (!newData)
            {
                free(mData);
                mData = nullptr;
                return 0;
            }
            mData = newData;
        }
    }

    memcpy((char*)mData + mUsed, pSrc, num);
    mUsed += num;
    return num;
}

} // namespace Common
} // namespace NFE
