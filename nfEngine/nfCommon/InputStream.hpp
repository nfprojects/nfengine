/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkkulagowski (mkulagowski(at)gmail.com)
 * @brief  Declaration of InputStream class for reading files, buffers, etc.
 */

#pragma once

#include "nfCommon.hpp"
#include "Language.hpp"
#include "File.hpp"

namespace NFE {
namespace Common {

/**
 * Abstract class used for reading files, buffers, etc.
 */
class NFCOMMON_API InputStream
{
public:
    virtual ~InputStream() { }

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
     * @param buffer Destination buffer.
     * @param num Number of bytes to be read.
     * @return Number of bytes read.
     */
    virtual size_t Read(void* buffer, size_t num) = 0;

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
class NFCOMMON_API FileInputStream : public NonCopyable, public InputStream
{
    File mFile;

public:
    FileInputStream(const char* path);
    ~FileInputStream();

    uint64 GetSize() override;
    bool Seek(uint64 position) override;
    size_t Read(void* buffer, size_t num) override;
};

/**
 * Implementation of InputStream - buffer (file in the memory) reader.
 */
class NFCOMMON_API BufferInputStream : public NonCopyable, public InputStream
{
    const void* mData;
    uint64 mSize;
    uint64 mPos;

public:
    BufferInputStream(const void* data, size_t dataSize);

    uint64 GetSize() override;
    bool Seek(uint64 position) override;
    size_t Read(void* buffer, size_t num) override;
};

} // namespace Common
} // namespace NFE
