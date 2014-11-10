/**
 * @file   OutputStream.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of OutputStream class for writing files, buffers, etc.
 */

#pragma once
#include "nfCommon.hpp"

namespace NFE {
namespace Common {

/**
 * Abstract class used for writing files, buffers, etc.
 */
class NFCOMMON_API OutputStream
{
public:
    virtual ~OutputStream();

    /**
     * Write data to the stream.
     * @param pSrc Source data buffer.
     * @param num Number of bytes to be written.
     * @return Number of written bytes.
     */
    virtual size_t Write(const void* pSrc, size_t num) = 0;

    /*
       TODO:
       * handling RW streams (additional class?)
       * integration with std::ostream
    */
};

/**
 * Implementation of OutputStream - file writer.
 */
class NFCOMMON_API FileOutputStream : public OutputStream
{
    HANDLE mFile;

    /// disable copy methods
    FileOutputStream(const FileOutputStream&);
    FileOutputStream& operator= (const FileOutputStream&);

public:
    FileOutputStream(const char* pFileName);
    FileOutputStream(const wchar_t* pFileName);
    ~FileOutputStream();

    size_t Write(const void* pSrc, size_t num);
};

/**
 * Implementation of OutputStream - buffer writer.
 */
class NFCOMMON_API BufferOutputStream : public OutputStream
{
private:
    void* mData;
    size_t mBufferSize;
    size_t mUsed;

public:
    BufferOutputStream();
    ~BufferOutputStream();

    size_t GetSize() const;
    const void* GetData() const;

    size_t Write(const void* pSrc, size_t num);

    void Clear();
};

} // namespace Common
} // namespace NFE
