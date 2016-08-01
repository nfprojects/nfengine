/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkkulagowski (mkulagowski(at)gmail.com)
 * @brief  Declaration of OutputStream class for writing files, buffers, etc.
 */

#pragma once

#include "nfCommon.hpp"
#include "Language.hpp"
#include "File.hpp"


namespace NFE {
namespace Common {

/**
 * Abstract class used for writing files, buffers, etc.
 */
class NFCOMMON_API OutputStream
{
public:
    virtual ~OutputStream() { }

    /**
     * Write data to the stream.
     * @param buffer Source data buffer.
     * @param num Number of bytes to be written.
     * @return Number of written bytes.
     */
    virtual size_t Write(const void* buffer, size_t num) = 0;

    /*
       TODO:
       * handling RW streams (additional class?)
       * integration with std::ostream
    */
};

/**
 * Implementation of OutputStream - file writer.
 */
class NFCOMMON_API FileOutputStream : public NonCopyable, public OutputStream
{
    File mFile;

public:
    FileOutputStream(const char* pFileName);

    size_t Write(const void* buffer, size_t num) override;
};

/**
 * Implementation of OutputStream - buffer writer.
 */
class NFCOMMON_API BufferOutputStream : public NonCopyable, public OutputStream
{
    void* mData;
    size_t mBufferSize;
    size_t mUsed;

public:
    BufferOutputStream();
    ~BufferOutputStream();

    size_t GetSize() const;
    const void* GetData() const;

    size_t Write(const void* buffer, size_t num) override;

    void Clear();
};

} // namespace Common
} // namespace NFE
