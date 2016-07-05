/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  File with defined bitwise operations
 */
#pragma once

#include "nfCommon.hpp"

namespace NFE
{
namespace Common
{
#define SWAP_ENDIANNESS(x) do {                                     \
        x = ((x & 0xFF000000) >> 24) | ((x & 0x00FF0000) >>  8) |   \
            ((x & 0x0000FF00) << 8)  | ((x & 0x000000FF) << 24);    \
    } while (0)

// Function to create continuous bitmask of given length
uint32 NFE_INLINE CreateBitMask(uint8 length)
{
    uint32 mask = 0;
    for (uint8 i = 0; i < length; i++)
        mask = (mask << 1) | 0x1;
    return mask;
}

/* Function used to count trailing zeros.
 * Taken from (https://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightParallel)
 */
uint8 NFE_INLINE CountTrailingZeros(uint32 bytes)
{
    uint32 v = bytes; // 32-bit word input to count zero bits on right
    uint8 c = 32;     // c will be the number of zero bits on the right

    v &= -static_cast<int32>(v);
    if (v) c--;
    if (v & 0x0000FFFF) c -= 16;
    if (v & 0x00FF00FF) c -= 8;
    if (v & 0x0F0F0F0F) c -= 4;
    if (v & 0x33333333) c -= 2;
    if (v & 0x55555555) c -= 1;

    return c;
}

} // namespace Common
} // namespace NFE