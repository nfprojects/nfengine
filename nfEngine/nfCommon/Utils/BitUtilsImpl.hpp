/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  File with defined bitwise operations
 */
#pragma once

#include "BitUtils.hpp"


namespace NFE {
namespace Common {

namespace {

// bit reverse table for 8-bit values

static const uint8 BIT_REVERSE_TABLE_256[] =
{
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

} // namespace

//////////////////////////////////////////////////////////////////////////

template<>
inline size_t CountBits(uint64 x)
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
inline size_t CountBits(uint32 x)
{
    x -= ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    return ((x + (x >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
}

template<>
inline size_t CountBits(uint16 x)
{
    // based on https://en.wikipedia.org/wiki/Hamming_weight

    const uint64 m1 = 0x5555;
    const uint64 m2 = 0x3333;
    const uint64 m4 = 0x0f0f;
    const uint64 m8 = 0x00ff;
    const uint64 hff = 0xffff;
    const uint64 h01 = 0x0101;

    x = (x & m1) + ((x >> 1) & m1);
    x = (x & m2) + ((x >> 2) & m2);
    x = (x & m4) + ((x >> 4) & m4);
    x = (x & m8) + ((x >> 8) & m8);
    return x;
}

template<>
inline size_t CountBits(uint8 x)
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
inline size_t CountLeadingZeros(uint64 x)
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
inline size_t CountLeadingZeros(uint32 x)
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
inline size_t CountLeadingZeros(uint16 x)
{
    size_t c = 0;
    if (x <= 0x00FF) c += 8, x <<= 8;
    if (x <= 0x0FFF) c += 4, x <<= 4;
    if (x <= 0x3FFF) c += 2, x <<= 2;
    if (x <= 0x7FFF) c++;
    return c;
}

template<>
inline size_t CountLeadingZeros(uint8 x)
{
    size_t c = 0;
    if (x <= 0x0F) c += 4, x <<= 4;
    if (x <= 0x3F) c += 2, x <<= 2;
    if (x <= 0x7F) c++;
    return c;
}


//////////////////////////////////////////////////////////////////////////

template<>
inline size_t CountTrailingZeros(const uint64 x)
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
inline size_t CountTrailingZeros(const uint32 x)
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
inline size_t CountTrailingZeros(const uint16 x)
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
inline size_t CountTrailingZeros(const uint8 x)
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
inline uint64 ReverseBits(uint64 x)
{
    return
        (static_cast<uint64>(BIT_REVERSE_TABLE_256[x & 0xFF]) << 56) |
        (static_cast<uint64>(BIT_REVERSE_TABLE_256[(x >> 8) & 0xFF]) << 48) |
        (static_cast<uint64>(BIT_REVERSE_TABLE_256[(x >> 16) & 0xFF]) << 40) |
        (static_cast<uint64>(BIT_REVERSE_TABLE_256[(x >> 24) & 0xFF]) << 32) |
        (static_cast<uint64>(BIT_REVERSE_TABLE_256[(x >> 32) & 0xFF]) << 24) |
        (static_cast<uint64>(BIT_REVERSE_TABLE_256[(x >> 40) & 0xFF]) << 16) |
        (static_cast<uint64>(BIT_REVERSE_TABLE_256[(x >> 48) & 0xFF]) << 8) |
        (static_cast<uint64>(BIT_REVERSE_TABLE_256[(x >> 56) & 0xFF]));
}

template<>
inline uint32 ReverseBits(uint32 x)
{
    return
        (BIT_REVERSE_TABLE_256[x & 0xFF] << 24) |
        (BIT_REVERSE_TABLE_256[(x >> 8) & 0xFF] << 16) |
        (BIT_REVERSE_TABLE_256[(x >> 16) & 0xFF] << 8) |
        (BIT_REVERSE_TABLE_256[(x >> 24) & 0xFF]);
}

template<>
inline uint16 ReverseBits(uint16 x)
{
    return
        (BIT_REVERSE_TABLE_256[x & 0xFF] << 8) |
        (BIT_REVERSE_TABLE_256[(x >> 8) & 0xFF]);

}


template<>
inline uint8 ReverseBits(uint8 x)
{
    return BIT_REVERSE_TABLE_256[x];

}

//////////////////////////////////////////////////////////////////////////

uint32 CreateBitMask(uint8 length)
{
    uint32 mask = 0;
    for (uint8 i = 0; i < length; i++)
        mask = (mask << 1) | 0x1;
    return mask;
}

//////////////////////////////////////////////////////////////////////////

template<>
inline uint16 SwapEndianness(uint16 x)
{
    return ((x >> 8) & 0x00FF) | ((x & 0x00FF) << 8);
}

template<>
inline uint32 SwapEndianness(uint32 x)
{
    // swap bytes
    x = ((x >> 8) & 0x00FF00FF) | ((x & 0x00FF00FF) << 8);
    // swap 2-byte long pairs
    x = (x >> 16) | (x << 16);

    return x;
}

template<>
inline uint64 SwapEndianness(uint64 x)
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