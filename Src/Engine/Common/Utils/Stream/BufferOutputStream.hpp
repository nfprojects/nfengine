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
    BufferOutputStream(Buffer& targetBuffer);
    ~BufferOutputStream();

    size_t GetSize() const;
    const void* GetData() const;

    virtual size_t Write(const void* buffer, size_t num) override;
    virtual bool Seek(uint64 position) override;

    void Clear();

private:
    Buffer& mBuffer;
    size_t mCursor;
};

} // namespace Common
} // namespace NFE
