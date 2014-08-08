#include "stdafx.h"
#include "InputStream.h"

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
    mFile = CreateFileA(pPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                        FILE_FLAG_SEQUENTIAL_SCAN, 0);
}

FileInputStream::FileInputStream(const wchar_t* pPath)
{
    mFile = CreateFileW(pPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                        FILE_FLAG_SEQUENTIAL_SCAN, 0);
}

FileInputStream::~FileInputStream()
{
    if (mFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(mFile);
        mFile = 0;
    }
}

uint64 FileInputStream::GetSize()
{
    if (mFile != INVALID_HANDLE_VALUE)
    {
        DWORD low, high;
        low = GetFileSize(mFile, &high);
        return (uint64)low | ((uint64)high << 32);
    }

    return 0;
}

// Jump to specific position in stream.
bool FileInputStream::Seek(uint64 position)
{
    if (mFile != INVALID_HANDLE_VALUE)
    {
        LONG high = position >> 32;
        SetFilePointer(mFile, (LONG)position, &high, FILE_BEGIN);
        return true;
    }

    return false;
}

// Fetch 'num' bytes and write to pDest. Returns number of bytes read.
size_t FileInputStream::Read(size_t num, void* pDest)
{
    if (mFile != INVALID_HANDLE_VALUE)
    {
        DWORD bytesRead = 0;
        ReadFile(mFile, pDest, num, &bytesRead, 0);
        return bytesRead;
    }

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
