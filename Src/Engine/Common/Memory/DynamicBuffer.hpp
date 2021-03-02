/**
 * @file
 * @author Witek902
 * @brief  DynamicBuffer class declaration.
 */

#pragma once

#include "BufferView.hpp"

namespace NFE {
namespace Common {

/**
 * Dynamic data buffer (like DynArray<uint8>)
 */
class NFCOMMON_API DynamicBuffer
{
public:

    DynamicBuffer();
    DynamicBuffer(const DynamicBuffer& src);
    DynamicBuffer(DynamicBuffer&& other);
    DynamicBuffer(const void* data, const size_t dataSize, const size_t alignment = 1);
    DynamicBuffer& operator = (const DynamicBuffer& src);
    DynamicBuffer& operator = (DynamicBuffer&& src);
    ~DynamicBuffer();

    NFE_FORCE_INLINE bool Empty() const { return mSize == 0; }
    NFE_FORCE_INLINE size_t Size() const { return mSize; }
    NFE_FORCE_INLINE size_t GetAlignment() const { return mAlignment; }
    NFE_FORCE_INLINE size_t Capacity() const { return mCapacity; }
    NFE_FORCE_INLINE void* Data() const { return mData; }

    NFE_FORCE_INLINE const BufferView ToView() const { return { mData, mSize }; }
    NFE_FORCE_INLINE operator BufferView() const { return { mData, mSize }; }

    // Fill with zeros
    void Zero();

    // Resize bufer and optionaly copy data into it
    bool Resize(size_t size, const void* data = nullptr, const size_t alignment = 1);

    // Reserve space
    bool Reserve(size_t size, bool preserveData = true, const size_t alignment = 1);

    // Set size to zero
    void Clear();

    // Free memory
    void Release();

private:
    void* mData;
    size_t mSize;
    size_t mCapacity;
    size_t mAlignment;
};

} // namespace Common
} // namespace NFE
