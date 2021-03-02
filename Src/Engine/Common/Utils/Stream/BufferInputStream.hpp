#pragma once

#include "InputStream.hpp"


namespace NFE {
namespace Common {

/**
 * Implementation of InputStream - buffer (file in the memory) reader.
 */
class NFCOMMON_API BufferInputStream : public InputStream
{
    NFE_MAKE_NONCOPYABLE(BufferInputStream)
    NFE_MAKE_NONMOVEABLE(BufferInputStream)

private:
    const void* mData;
    size_t mSize;
    size_t mPos;

public:
    BufferInputStream(const void* data, size_t dataSize);
    BufferInputStream(const BufferView& buffer);

    uint64 GetSize() override;
    bool Seek(int64 offset, SeekMode mode) override;
    size_t Read(void* buffer, size_t num) override;
};

} // namespace Common
} // namespace NFE
