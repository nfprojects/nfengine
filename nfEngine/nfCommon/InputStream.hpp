/**
    NFEngine project

    \file   InputStream.hpp
*/

#pragma once
#include "nfCommon.hpp"

namespace NFE {
namespace Common {

/**
 * Abstract class used for reading files, buffers, etc.
 */
class NFCOMMON_API InputStream
{
public:
    virtual ~InputStream();

    /**
     * Get stream size.
     * @return Stream size in bytes.
     */
    virtual uint64 GetSize() = 0;

    /**
     * Jump to specific position in stream.
     * @return true on success.
     */
    virtual bool Seek(uint64 position) = 0;

    /**
     * Fetch bytes and store in external buffer.
     * @param num Number of bytes to be read.
     * @param pDest Destination buffer.
     * @return Number of bytes read.
     */
    virtual size_t Read(size_t num, void* pDest) = 0;

    /*
       TODO:
       * relative, absolute seeking
       * handling infinite streams (GetSize() == inf)
       * integration with std::istream
    */
};

/**
 * Implementation of InputStream - file reader.
 */
class NFCOMMON_API FileInputStream : public InputStream
{
    HANDLE mFile;

    /// disable copy methods
    FileInputStream(const FileInputStream&);
    FileInputStream& operator= (const FileInputStream&);

public:
    FileInputStream(const char* pPath);
    FileInputStream(const wchar_t* pPath);
    ~FileInputStream();

    uint64 GetSize();
    bool Seek(uint64 position);
    size_t Read(size_t num, void* pDest);
};

/**
 * Implementation of InputStream - buffer (file in the memory) reader.
 */
class NFCOMMON_API BufferInputStream : public InputStream
{
    const void* mData;
    uint64 mSize;
    uint64 mPos;

    /// disable copy methods
    BufferInputStream(const BufferInputStream&);
    BufferInputStream& operator= (const BufferInputStream&);

public:
    BufferInputStream(const void* pData, size_t dataSize);
    ~BufferInputStream();

    uint64 GetSize();
    bool Seek(uint64 position);
    size_t Read(size_t num, void* pDest);
};

} // namespace Common
} // namespace NFE
