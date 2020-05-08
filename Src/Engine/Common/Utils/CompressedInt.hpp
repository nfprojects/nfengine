/**
 * @file    Witek902
 * @brief   Variable-lenght coding of integer numbers
 */

#pragma once

#include "../nfCommon.hpp"
#include "../System/Assertion.hpp"

namespace NFE {
namespace Common {

template<typename IntType = uint32>
struct CompressedUint
{
    static_assert(std::is_unsigned_v<IntType>, "Invalid compressed uint type");
    static_assert(sizeof(IntType) <= 10, "Invalid compressed uint type");

    static constexpr uint32 MaxBufferLength = 10;

    using BufferType = uint8[MaxBufferLength];


    static uint32 Encode(IntType value, BufferType outputBuffer)
    {
        uint32 outputSize = 0;
        while (value > 127)
        {
            // set the next byte flag
            outputBuffer[outputSize] = ((uint8)(value & 0x7Fu)) | 0x80u;

            // Remove the seven bits we just wrote
            value >>= 7;
            outputSize++;
        }
        outputBuffer[outputSize++] = static_cast<uint8>(value) & 0x7Fu;

        NFE_ASSERT(outputSize <= MaxBufferLength);
        return outputSize;
    }

    static bool Decode(BufferType inputBuffer, uint32 inputBufferSize, IntType& outValue)
    {
        IntType ret = 0;
        for (uint32 i = 0; i < inputBufferSize; i++)
        {
            ret |= (IntType)(inputBuffer[i] & 0x7Fu) << (7u * i);

            // next-byte flag is set
            if (0 == (inputBuffer[i] & 0x80u))
            {
                outValue = ret;
                return true;
            }
        }

        // no cleared "next byte flag" found - the encoding is invalid
        return false;
    }

};


} // namespace Common
} // namespace NFE
