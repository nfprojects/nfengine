/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  File with AVX implementation of bit operations.
 */
#pragma once

#include "BitUtils.hpp"
#include "../Math/Math.hpp"


#ifdef NFE_USE_AVX
    #ifndef NFE_USE_SSE
        #error "SSE must be enabled when using AVX"
    #endif // NFE_USE_SSE
#include <immintrin.h>
#endif

namespace NFE {
namespace Common {


template<>
inline size_t BitUtils<uint64>::CountBits(uint64 x)
{
#if defined(_M_X64) || defined(__amd64__)
    return __popcnt64(x);
#else
    return __popcnt(x & 0xFFFFFFFF) + __popcnt(x >> 32);
#endif  
}

template<>
inline size_t BitUtils<uint32>::CountBits(uint32 x)
{
    return __popcnt(x);
}

template<>
inline size_t BitUtils<uint16>::CountBits(uint16 x)
{
    return __popcnt16(x);
}

template<>
inline size_t BitUtils<uint8>::CountBits(uint8 x)
{
    return __popcnt16(x);
}


//////////////////////////////////////////////////////////////////////////


template<>
NFE_INLINE size_t BitUtils<uint64>::CountLeadingZeros(uint64 x)
{
#if defined(_M_X64) || defined(__amd64__)
    unsigned long index = 0;
    _BitScanReverse64(&index, x);
    return 63 - index;
#else
    unsigned long index = 0;

    // find '1' in upper half
    const uint32 hi = static_cast<uint32>(x >> 32);
    if (_BitScanReverse(&index, hi))
    {
        return 31 - index;
    }

    // find '1' in lower half
    const uint32 lo = static_cast<uint32>(x & 0xFFFFFFFFull);
    _BitScanReverse(&index, lo);
    return 32 + 31 - index;
#endif  
}

template<>
NFE_INLINE size_t BitUtils<uint32>::CountLeadingZeros(uint32 x)
{
    unsigned long index = 0;
    _BitScanReverse(&index, x);
    return 31 - index;
}

template<>
NFE_INLINE size_t BitUtils<uint16>::CountLeadingZeros(uint16 x)
{
    unsigned long index = 0;
    _BitScanReverse(&index, x);
    return 15 - index;
}

template<>
NFE_INLINE size_t BitUtils<uint8>::CountLeadingZeros(uint8 x)
{
    unsigned long index = 0;
    _BitScanReverse(&index, x);
    return 7 - index;
}


//////////////////////////////////////////////////////////////////////////


template<>
NFE_INLINE size_t BitUtils<uint64>::CountTrailingZeros(const uint64 x)
{
#if defined(_M_X64) || defined(__amd64__)
    unsigned long index = 0;
    _BitScanForward64(&index, x);
    return index;
#else
    unsigned long index = 0;

    // find '1' in lower half
    const uint32 lo = static_cast<uint32>(x & 0xFFFFFFFFull);
    if (_BitScanForward(&index, lo))
    {
        return index;
    }

    // find '1' in upper half
    const uint32 hi = static_cast<uint32>(x >> 32);
    _BitScanForward(&index, hi);
    return 32 + index;
#endif  
}

template<>
NFE_INLINE size_t BitUtils<uint32>::CountTrailingZeros(const uint32 x)
{
    unsigned long index = 0;
    _BitScanForward(&index, x);
    return index;
}

template<>
NFE_INLINE size_t BitUtils<uint16>::CountTrailingZeros(const uint16 x)
{
    unsigned long index = 0;
    _BitScanForward(&index, x);
    return index;
}

template<>
NFE_INLINE size_t BitUtils<uint8>::CountTrailingZeros(const uint8 x)
{
    unsigned long index = 0;
    _BitScanForward(&index, x);
    return index;
}


//////////////////////////////////////////////////////////////////////////


template<>
NFE_INLINE uint16 BitUtils<uint16>::SwapEndianness(uint16 x)
{
    return _byteswap_ushort(x);
}

template<>
NFE_INLINE uint32 BitUtils<uint32>::SwapEndianness(uint32 x)
{
    return _byteswap_ulong(x);
}

template<>
NFE_INLINE uint64 BitUtils<uint64>::SwapEndianness(uint64 x)
{
    return _byteswap_uint64(x);
}

} // namespace Common
} // namespace NFE