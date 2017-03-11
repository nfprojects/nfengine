/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  File with defined native bitwise operations
 */
#pragma once

#include "BitUtils.hpp"
#include "../Math/Math.hpp"


namespace NFE {
namespace Common {


template<>
inline size_t BitUtils<uint64>::CountBits(uint64 x)
{
    // based on https://en.wikipedia.org/wiki/Hamming_weight

    const uint64 m1 = 0x5555555555555555;
    const uint64 m2 = 0x3333333333333333;
    const uint64 m4 = 0x0f0f0f0f0f0f0f0f;
    const uint64 h01 = 0x0101010101010101;

    x -= (x >> 1) & m1;
    x = (x & m2) + ((x >> 2) & m2);
    x = (x + (x >> 4)) & m4;
    return (x * h01) >> 56;
}

template<>
inline size_t BitUtils<uint32>::CountBits(uint32 x)
{
    x -= ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    return (((x + (x >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
}

template<>
inline size_t BitUtils<uint16>::CountBits(uint16 x)
{
    // based on https://en.wikipedia.org/wiki/Hamming_weight

    const uint64 m1 = 0x5555;
    const uint64 m2 = 0x3333;
    const uint64 m4 = 0x0f0f;
    const uint64 m8 = 0x00ff;

    x = (x & m1) + ((x >> 1) & m1);
    x = (x & m2) + ((x >> 2) & m2);
    x = (x & m4) + ((x >> 4) & m4);
    x = (x & m8) + ((x >> 8) & m8);
    return x;
}

template<>
inline size_t BitUtils<uint8>::CountBits(uint8 x)
{
    // based on https://en.wikipedia.org/wiki/Hamming_weight

    const uint64 m1 = 0x55;
    const uint64 m2 = 0x33;
    const uint64 m4 = 0x0f;

    x = (x & m1) + ((x >> 1) & m1);
    x = (x & m2) + ((x >> 2) & m2);
    x = (x & m4) + ((x >> 4) & m4);
    return x;
}

//////////////////////////////////////////////////////////////////////////

template<>
inline size_t BitUtils<uint64>::CountLeadingZeros(uint64 x)
{
    size_t c = 0;
    if (x <= 0x00000000FFFFFFFFul) c += 32, x <<= 32;
    if (x <= 0x0000FFFFFFFFFFFFul) c += 16, x <<= 16;
    if (x <= 0x00FFFFFFFFFFFFFFul) c += 8, x <<= 8;
    if (x <= 0x0FFFFFFFFFFFFFFFul) c += 4, x <<= 4;
    if (x <= 0x3FFFFFFFFFFFFFFFul) c += 2, x <<= 2;
    if (x <= 0x7FFFFFFFFFFFFFFFul) c++;
    return c;
}

template<>
inline size_t BitUtils<uint32>::CountLeadingZeros(uint32 x)
{
    size_t c = 0;
    if (x <= 0x0000FFFF) c += 16, x <<= 16;
    if (x <= 0x00FFFFFF) c += 8, x <<= 8;
    if (x <= 0x0FFFFFFF) c += 4, x <<= 4;
    if (x <= 0x3FFFFFFF) c += 2, x <<= 2;
    if (x <= 0x7FFFFFFF) c++;
    return c;
}

template<>
inline size_t BitUtils<uint16>::CountLeadingZeros(uint16 x)
{
    size_t c = 0;
    if (x <= 0x00FF) c += 8, x <<= 8;
    if (x <= 0x0FFF) c += 4, x <<= 4;
    if (x <= 0x3FFF) c += 2, x <<= 2;
    if (x <= 0x7FFF) c++;
    return c;
}

template<>
inline size_t BitUtils<uint8>::CountLeadingZeros(uint8 x)
{
    size_t c = 0;
    if (x <= 0x0F) c += 4, x <<= 4;
    if (x <= 0x3F) c += 2, x <<= 2;
    if (x <= 0x7F) c++;
    return c;
}


//////////////////////////////////////////////////////////////////////////

template<>
inline size_t BitUtils<uint64>::CountTrailingZeros(const uint64 x)
{
    // based on:
    // https://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightParallel

    uint64 v = x;   // 32-bit word input to count zero bits on right
    size_t c = 64;  // c will be the number of zero bits on the right

    v &= -static_cast<int64>(v);
    if (v) c--;
    if (v & 0x00000000FFFFFFFFul) c -= 32;
    if (v & 0x0000FFFF0000FFFFul) c -= 16;
    if (v & 0x00FF00FF00FF00FFul) c -= 8;
    if (v & 0x0F0F0F0F0F0F0F0Ful) c -= 4;
    if (v & 0x3333333333333333ul) c -= 2;
    if (v & 0x5555555555555555ul) c -= 1;
    return c;
}

template<>
inline size_t BitUtils<uint32>::CountTrailingZeros(const uint32 x)
{
    // based on:
    // https://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightParallel

    uint32 v = x;   // 32-bit word input to count zero bits on right
    size_t c = 32;  // c will be the number of zero bits on the right

    v &= -static_cast<int32>(v);
    if (v) c--;
    if (v & 0x0000FFFF) c -= 16;
    if (v & 0x00FF00FF) c -= 8;
    if (v & 0x0F0F0F0F) c -= 4;
    if (v & 0x33333333) c -= 2;
    if (v & 0x55555555) c -= 1;
    return c;
}

template<>
inline size_t BitUtils<uint16>::CountTrailingZeros(const uint16 x)
{
    // based on:
    // https://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightParallel

    uint16 v = x;   // 32-bit word input to count zero bits on right
    size_t c = 16;  // c will be the number of zero bits on the right

    v &= -static_cast<int16>(v);
    if (v) c--;
    if (v & static_cast<uint16>(0x00FF)) c -= 8;
    if (v & static_cast<uint16>(0x0F0F)) c -= 4;
    if (v & static_cast<uint16>(0x3333)) c -= 2;
    if (v & static_cast<uint16>(0x5555)) c -= 1;
    return c;
}

template<>
inline size_t BitUtils<uint8>::CountTrailingZeros(const uint8 x)
{
    // based on:
    // https://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightParallel

    uint8 v = x;    // 32-bit word input to count zero bits on right
    size_t c = 8;   // c will be the number of zero bits on the right

    v &= -static_cast<int8>(v);
    if (v) c--;
    if (v & static_cast<uint8>(0x0F)) c -= 4;
    if (v & static_cast<uint8>(0x33)) c -= 2;
    if (v & static_cast<uint8>(0x55)) c -= 1;

    return c;
}

//////////////////////////////////////////////////////////////////////////

template<>
inline uint16 BitUtils<uint16>::SwapEndianness(uint16 x)
{
    return ((x >> 8) & 0x00FF) | ((x & 0x00FF) << 8);
}

template<>
inline uint32 BitUtils<uint32>::SwapEndianness(uint32 x)
{
    // swap bytes
    x = ((x >> 8) & 0x00FF00FF) | ((x & 0x00FF00FF) << 8);
    // swap 2-byte long pairs
    x = (x >> 16) | (x << 16);
    return x;
}

template<>
inline uint64 BitUtils<uint64>::SwapEndianness(uint64 x)
{
    // swap bytes
    x = ((x >> 8) & 0x00FF00FF00FF00FFull) | ((x & 0x00FF00FF00FF00FFull) << 8);
    // swap 2-byte long pairs
    x = ((x >> 16) & 0x0000FFFF0000FFFFull) | ((x & 0x0000FFFF0000FFFFull) << 16);
    // swap 4-byte long pairs
    x = (x >> 32) | (x << 32);
    return x;
}

} // namespace Common
} // namespace NFE