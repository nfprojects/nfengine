/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkkulagowski (mkulagowski(at)gmail.com)
 * @brief  Declaration of OutputStream class for writing files, buffers, etc.
 */

#pragma once

#include "../CompressedInt.hpp"
#include "StreamCommon.hpp"

namespace NFE {
namespace Common {

/**
 * Abstract class used for writing files, buffers, etc.
 */
class NFCOMMON_API OutputStream
{
public:
    OutputStream();
    virtual ~OutputStream();

    // Write data to the stream
    virtual size_t Write(const void* buffer, size_t num) = 0;

    // Get current write cursor position
    virtual uint64 GetPosition() const = 0;

    // Set write cursor position
    virtual bool Seek(int64 offset, SeekMode mode) = 0;

    template<typename T>
    NFE_FORCE_INLINE bool Write(const T& data)
    {
        static_assert(!std::is_pointer_v<T>, "Do not serialize pointers");

        return Write(&data, sizeof(T)) == sizeof(T);
    }

    template<typename IntType>
    bool WriteCompressedUint(const IntType value)
    {
        static_assert(std::is_unsigned_v<IntType>, "Invalid compressed uint type");

        typename CompressedUint<IntType>::BufferType buffer;
        const uint32 bufferSize = CompressedUint<IntType>::Encode(value, buffer);

        return Write(buffer, bufferSize) == bufferSize;
    }

    template<typename IntType>
    bool WriteCompressedPositiveInt(const IntType value)
    {
        NFE_ASSERT(value > 0u, "Value must be positive");
        return WriteCompressedUint(value - 1u);
    }
};

} // namespace Common
} // namespace NFE
