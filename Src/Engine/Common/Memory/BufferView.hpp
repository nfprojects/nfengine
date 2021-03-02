/**
 * @file
 * @author Witek902
 * @brief  BufferView class declaration.
 */

#pragma once

#include "../nfCommon.hpp"


namespace NFE {
namespace Common {

/**
 * View of data buffer (like ArrayView<uint8>)
 */
class NFCOMMON_API BufferView final
{
public:

    NFE_FORCE_INLINE BufferView() : mData(nullptr), mSize(0) { }
    NFE_FORCE_INLINE BufferView(void* data, const size_t dataSize) : mData(data), mSize(dataSize) { }

    NFE_FORCE_INLINE BufferView(const BufferView& other) = default;
    NFE_FORCE_INLINE BufferView& operator = (const BufferView& src) = default;

    NFE_FORCE_INLINE bool Empty() const { return mSize == 0u; }

    NFE_FORCE_INLINE size_t Size() const { return mSize; }

    NFE_FORCE_INLINE void* Data() const { return mData; }

    friend NFCOMMON_API bool operator == (const BufferView& lhs, const BufferView& rhs);
    friend NFCOMMON_API bool operator != (const BufferView& lhs, const BufferView& rhs);

private:
    void* mData;
    size_t mSize;
};

} // namespace Common
} // namespace NFE
