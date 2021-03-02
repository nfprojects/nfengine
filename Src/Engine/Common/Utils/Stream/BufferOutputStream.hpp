#pragma once

#include "OutputStream.hpp"


namespace NFE {
namespace Common {

/**
 * Implementation of OutputStream - buffer writer.
 */
class NFCOMMON_API BufferOutputStream : public OutputStream
{
    NFE_MAKE_NONCOPYABLE(BufferOutputStream)
    NFE_MAKE_NONMOVEABLE(BufferOutputStream)

public:
    BufferOutputStream(DynamicBuffer& targetBuffer);
    ~BufferOutputStream();

    size_t GetSize() const;
    const void* GetData() const;

    virtual size_t Write(const void* buffer, size_t num) override;
    virtual bool Seek(int64 offset, SeekMode mode) override;
    virtual uint64 GetPosition() const override;

private:
    DynamicBuffer& mBuffer;
    size_t mCursor;
};

} // namespace Common
} // namespace NFE
