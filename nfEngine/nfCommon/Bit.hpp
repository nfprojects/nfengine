/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  File with defined bitwise operations
 */
#pragma once

#define SWAP_ENDIANNESS(x) do {                                     \
        x = ((x & 0xFF000000) >> 24) | ((x & 0x00FF0000) >>  8) |   \
            ((x & 0x0000FF00) << 8)  | ((x & 0x000000FF) << 24);    \
    } while (0)

// Function to create continuous bitmask of given length
uint32_t NFE_INLINE CreateBitMask(uint8_t length)
{
    uint32_t mask = 0;
    for (uint8_t i = 0; i < length; i++)
        mask = (mask << 1) | 0x1;
    return mask;
}

/* Function used to count trailing zeros.
 * Taken from (https://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightParallel)
 */
uint8_t NFE_INLINE CountTrailingZeros(uint32_t bytes)
{
    uint32_t v = bytes; // 32-bit word input to count zero bits on right
    uint8_t c = 32;     // c will be the number of zero bits on the right

    v &= -static_cast<int32_t>(v);
    if (v) c--;
    if (v & 0x0000FFFF) c -= 16;
    if (v & 0x00FF00FF) c -= 8;
    if (v & 0x0F0F0F0F) c -= 4;
    if (v & 0x33333333) c -= 2;
    if (v & 0x55555555) c -= 1;

    return c;
}
