/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkkulagowski (mkulagowski(at)gmail.com)
 * @brief  Definition of InputStream class for reading files, buffers, etc.
 */

#include "PCH.hpp"
#include "InputStream.hpp"
#include "Memory.hpp"
#include "Logger.hpp"


namespace NFE {
namespace Common {

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

bool FileInputStream::Seek(uint64 position)
{
    return mFile.Seek(position, SeekMode::Begin);
}

size_t FileInputStream::Read(void* buffer, size_t num)
{
    return mFile.Read(buffer, num);
}


//-----------------------------------------------------------
// BufferInputStream
//-----------------------------------------------------------

BufferInputStream::BufferInputStream(const void* data, size_t dataSize)
{
    if (!MemoryCheck(data, dataSize))
    {
        LOG_ERROR("Provided buffer can't be read");
        mData = nullptr;
        mSize = 0;
    }

    mData = data;
    mSize = dataSize;
    mPos = 0;
}

uint64 BufferInputStream::GetSize()
{
    return mSize;
}

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

size_t BufferInputStream::Read(void* buffer, size_t num)
{
    if (!mData)
        return 0;

    if (mPos + num > mSize)
    {
        size_t maxReadable = static_cast<size_t>(mSize - mPos);
        memcpy(buffer, reinterpret_cast<const char*>(mData) + mPos, maxReadable);
        mPos = mSize;
        return maxReadable;
    }
    else
    {
        memcpy(buffer, reinterpret_cast<const char*>(mData) + mPos, num);
        mPos += num;
        return num;
    }
}

} // namespace Common
} // namespace NFE
