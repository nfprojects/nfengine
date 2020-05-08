/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkkulagowski (mkulagowski(at)gmail.com)
 * @brief  Declaration of InputStream class for reading files, buffers, etc.
 */

#pragma once

#include "../CompressedInt.hpp"

namespace NFE {
namespace Common {

/**
 * Abstract class used for reading files, buffers, etc.
 */
class NFCOMMON_API InputStream
{
public:
    InputStream();
    virtual ~InputStream();

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

    template<typename T>
    NFE_FORCE_INLINE bool Read(T& outData)
    {
        static_assert(!std::is_pointer_v<T>, "Do not serialize pointers");

        return Read(&outData, sizeof(T)) == sizeof(T);
    }

    template<typename IntType>
    bool ReadCompressedUint(IntType& outValue)
    {
        static_assert(std::is_unsigned_v<IntType>, "Invalid compressed uint type");

        IntType ret = 0;
        for (uint32 i = 0; i < CompressedUint<IntType>::MaxBufferLength; i++)
        {
            uint8 byte;
            if (!Read(byte))
            {
                // end of stream
                return false;
            }

            ret |= (IntType)(byte & 0x7Fu) << (7u * i);

            // next-byte flag is set
            if (0 == (byte & 0x80u))
            {
                outValue = ret;
                return true;
            }
        }

        // no cleared "next byte flag" found - the encoding is invalid
        return false;
    }

    template<typename IntType>
    NFE_FORCE_INLINE bool ReadCompressedPositiveInt(IntType& outValue)
    {
        IntType value = 0;
        const bool result = ReadCompressedUint(value);
        outValue = value + 1u;
        return result;
    }

    /*
       TODO:
       * relative, absolute seeking
       * handling infinite streams (GetSize() == inf)
       * integration with std::istream
    */
};

} // namespace Common
} // namespace NFE
