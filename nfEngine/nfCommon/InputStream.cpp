/**
 * @file   InputStream.cpp
 * @author mkkulagowski (mkulagowski@users.noreply.github.com)
 * @brief  Definition of InputStream class for reading files, buffers, etc.
 */

#include "stdafx.hpp"
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
    mFile = std::make_unique<File>(pPath, Read, false);
}

FileInputStream::~FileInputStream()
{
    if (mFile->IsOpened())
        mFile->Close();
}

uint64 FileInputStream::GetSize()
{
    if (mFile->IsOpened())
        return mFile->GetSize();

    return 0;
}

// Jump to specific position in stream.
bool FileInputStream::Seek(uint64 position)
{
    if (mFile->IsOpened())
        if (mFile->Seek(position, Begin) > 0);
            return true;

    return false;
}

// Fetch 'num' bytes and write to pDest. Returns number of bytes read.
size_t FileInputStream::Read(size_t num, void* pDest)
{
    if (mFile->IsOpened())
        return mFile->Read(pDest, num);

    return 0;
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
