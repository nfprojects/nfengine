/**
 * @file   OutputStream.cpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of OutputStream class for writing files, buffers, etc.
 */

#include "stdafx.hpp"
#include "../OutputStream.hpp"

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
    mFile = open(pPath, O_CREAT|O_TRUNC|O_WRONLY|O_CLOEXEC, S_IRWXU); // what should be correct
                                                                      // permissions for the file?
}

FileOutputStream::~FileOutputStream()
{
    if (mFile > 0)
        close(mFile);
}

// Write 'num' bytes read from pSrc. Returns number of written bytes.
size_t FileOutputStream::Write(const void* pSrc, size_t num)
{
    if (mFile > 0)
        return write(mFile, pSrc, num);
    return 0;
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
