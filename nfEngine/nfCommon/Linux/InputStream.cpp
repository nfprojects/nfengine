/**
 * @file   InputStream.cpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of InputStream class for reading files, buffers, etc.
 */

#include "stdafx.hpp"
#include "../InputStream.hpp"

namespace NFE {
namespace Common {

InputStream::~InputStream()
{
}



//-----------------------------------------------------------
// FileInputStream
//-----------------------------------------------------------

FileInputStream::FileInputStream(const char* pPath)
{
    mFile = open(pPath, O_RDONLY);
}

FileInputStream::~FileInputStream()
{
    if (mFile > 0)
        mFile = close(mFile);
}

uint64 FileInputStream::GetSize()
{
    if (mFile > 0)
    {
        struct stat statbuf;
        if (fstat(mFile, &statbuf) != -1)
          return static_cast<uint64>(statbuf.st_size); // if the value is different than expected,
                                                        // try st_blocks*512
    }
    return 0;
}

// Jump to specific position in stream.
bool FileInputStream::Seek(uint64 position)
{
    if (mFile > 0)
    {
        if (position > this->GetSize())
            return false;
        if (lseek(mFile, SEEK_SET, static_cast<int>(position)) != -1); // check without static_cast
            return true;
    }
    return false;
}

// Fetch 'num' bytes and write to pDest. Returns number of bytes read.
size_t FileInputStream::Read(size_t num, void* pDest)
{
    if (mFile > 0)
        return read(mFile, pDest, num);
    return 0;
}




//-----------------------------------------------------------
// BufferInputStream
//-----------------------------------------------------------

BufferInputStream::BufferInputStream(const void* pData, size_t dataSize)
{
    mData = pData;
    mSize = dataSize;
    mPos = 0;
}

BufferInputStream::~BufferInputStream()
{
    //nothing to free
}

// Get stream size (in bytes).
uint64 BufferInputStream::GetSize()
{
    return mSize;
}

// Jump to specific position in stream.
bool BufferInputStream::Seek(uint64 position)
{
    if (!mData)
        return false;

    if (position < mSize)
    {
        mPos = position;
        return true;
    }

    return false;
}

// Fetch 'num' bytes and write to pDest. Returns number of bytes read.
size_t BufferInputStream::Read(size_t num, void* pDest)
{
    if (!mData)
        return 0;

    if (mPos + num > mSize)
    {
        size_t maxReadable = (size_t)(mSize - mPos);
        memcpy(pDest, ((const char*)mData) + mPos, maxReadable);
        mPos = mSize;
        return maxReadable;
    }
    else
    {
        memcpy(pDest, ((const char*)mData) + mPos, num);
        mPos += num;
        return num;
    }
}

} // namespace Common
} // namespace NFE
