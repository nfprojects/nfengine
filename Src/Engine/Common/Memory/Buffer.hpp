/**
 * @file
 * @author Witek902
 * @brief  Buffer class declaration.
 */

#pragma once

#include "../nfCommon.hpp"


namespace NFE {
namespace Common {

/**
 * Dynamic data buffer
 */
class NFCOMMON_API Buffer
{
public:

    // TODO
    const uint32 Alignment = 1;

    Buffer();
    Buffer(const Buffer& src);
    Buffer(Buffer&& other);
    Buffer& operator = (const Buffer& src);
    Buffer& operator = (Buffer&& src);
    ~Buffer();

    NFE_FORCE_INLINE bool Empty() const { return mSize == 0; }
    NFE_FORCE_INLINE size_t Size() const { return mSize; }
    NFE_FORCE_INLINE size_t Capacity() const { return mCapacity; }
    NFE_FORCE_INLINE void* Data() const { return mData; }

    void Zero();

    // Resize bufer and optionaly copy data into it
    bool Resize(size_t size, const void* data = nullptr);

    // Reserve space
    bool Reserve(size_t size, bool preserveData = true);

    // Set size to zero
    void Clear();

    // Free memory
    void Release();

private:
    void* mData;
    size_t mSize;
    size_t mCapacity;
};

} // namespace Common
} // namespace NFE
