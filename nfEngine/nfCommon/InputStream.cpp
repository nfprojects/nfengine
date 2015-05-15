/**
 * @file   InputStream.cpp
 * @author Witek902 (witek902@gmail.com)
 * @author mkkulagowski (mkulagowski(at)gmail.com)
 * @brief  Definition of InputStream class for reading files, buffers, etc.
 */

#include "PCH.hpp"
#include "InputStream.hpp"

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
    mFile.Open(pPath, AccessMode::Read, false);
}

FileInputStream::~FileInputStream()
{
    mFile.Close();
}

uint64 FileInputStream::GetSize()
{
    return mFile.GetSize();
}

// Jump to specific position in stream.
bool FileInputStream::Seek(uint64 position)
{
    return mFile.Seek(position, SeekMode::Begin);
}

// Fetch 'num' bytes and write to pDest. Returns number of bytes read.
size_t FileInputStream::Read(size_t num, void* pDest)
{
    return mFile.Read(pDest, num);
}


//-----------------------------------------------------------
// BufferInputStream
//-----------------------------------------------------------

BufferInputStream::BufferInputStream(const void* pData, size_t dataSize)
{
    // TODO
    /*
    if (XMemoryCheck(pData, dataSize) & X_ACCESS_READ)
    {
        mpData = pData;
        mSize = dataSize;
    }
    else
    {
        mpData = 0;
        mSize = 0;
    }
    */

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

// Fetch 'bum' bytes and write to pDest. Returns number of bytes read.
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
