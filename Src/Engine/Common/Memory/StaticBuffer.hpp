/**
 * @file
 * @author Witek902
 * @brief  StaticBuffer class declaration.
 */

#pragma once

#include "BufferView.hpp"

namespace NFE {
namespace Common {

/**
 * Static data buffer (like UniquePtr<uint8[]>)
 */
class NFCOMMON_API StaticBuffer
{
public:

    StaticBuffer();
    StaticBuffer(const StaticBuffer& src);
    StaticBuffer(StaticBuffer&& other);
    StaticBuffer(const void* data, const size_t dataSize, const size_t alignment = 1);
    StaticBuffer& operator = (const StaticBuffer& src);
    StaticBuffer& operator = (StaticBuffer&& src);
    ~StaticBuffer();

    NFE_FORCE_INLINE bool Empty() const { return mSize == 0; }
    NFE_FORCE_INLINE size_t Size() const { return mSize; }
    NFE_FORCE_INLINE void* Data() const { return mData; }

    NFE_FORCE_INLINE const BufferView ToView() const { return { mData, mSize }; }

    // Fill with zeros
    void Zero();

    // Resize bufer and optionaly copy data into it
    bool Set(size_t size, const void* data = nullptr, const size_t alignment = 1);

    // Set size to zero (free memory)
    void Release();

private:
    void* mData;
    size_t mSize;
};

} // namespace Common
} // namespace NFE
