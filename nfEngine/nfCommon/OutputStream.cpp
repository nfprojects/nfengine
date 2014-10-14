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
    mFile = CreateFileA(pFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, 0);
}

FileOutputStream::FileOutputStream(const wchar_t* pFileName)
{
    mFile = CreateFileW(pFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, 0);
}

FileOutputStream::~FileOutputStream()
{
    if (mFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(mFile);
    }
}

// Write 'num' bytes read from pSrc. Returns number of written bytes.
size_t FileOutputStream::Write(const void* pSrc, size_t num)
{
    if (mFile != INVALID_HANDLE_VALUE)
    {
        DWORD written;
        if (WriteFile(mFile, pSrc, num, &written, 0))
        {
            return (size_t)written;
        }
    }

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
        if (!mData) return 0; //malloc failed

        mUsed = 0;
    }
    else
    {
        size_t minSize = mUsed + num;
        if (mBufferSize < minSize)
        {
            while (mBufferSize < minSize)
                mBufferSize *= 2;

            mData = realloc(mData, mBufferSize); // TODO: fix memory leak on realloc failure
            if (!mData) return 0; //realloc failed
        }
    }

    memcpy((char*)mData + mUsed, pSrc, num);
    mUsed += num;
    return num;
}

} // namespace Common
} // namespace NFE
