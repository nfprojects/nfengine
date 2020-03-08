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
    return __builtin_popcountll(x);
#else
    return __builtin_popcountl(x & 0xFFFFFFFF) + __builtin_popcountl(x >> 32);
#endif  
}

template<>
inline size_t BitUtils<uint32>::CountBits(uint32 x)
{
    return __builtin_popcountl(x);
}

template<>
inline size_t BitUtils<uint16>::CountBits(uint16 x)
{
    return __builtin_popcountl(static_cast<int>(x));
}

template<>
inline size_t BitUtils<uint8>::CountBits(uint8 x)
{
    return __builtin_popcountl(static_cast<int>(x));
}


//////////////////////////////////////////////////////////////////////////


template<>
NFE_INLINE size_t BitUtils<uint64>::CountLeadingZeros(uint64 x)
{
#if defined(_M_X64) || defined(__amd64__)
    return __builtin_clzll(x);
#else
    unsigned long index = 0;

    // find '1' in upper half
    const uint32 hi = static_cast<uint32>(x >> 32);
    if (__builtin_clzl(&index, hi))
    {
        return 31 - index;
    }

    // find '1' in lower half
    const uint32 lo = static_cast<uint32>(x & 0xFFFFFFFFull);
    __builtin_clzl(&index, lo);
    return 32 + 31 - index;
#endif  
}

template<>
NFE_INLINE size_t BitUtils<uint32>::CountLeadingZeros(uint32 x)
{
    return __builtin_clz(x);
}

template<>
NFE_INLINE size_t BitUtils<uint16>::CountLeadingZeros(uint16 x)
{
    const uint32 xExtended = (static_cast<uint32>(x) << 16) | 0x0000FFFFu;
    return __builtin_clz(xExtended);
}

template<>
NFE_INLINE size_t BitUtils<uint8>::CountLeadingZeros(uint8 x)
{
    const uint32 xExtended = (static_cast<uint32>(x) << 24) | 0x00FFFFFFu;
    return __builtin_clz(xExtended);
}


//////////////////////////////////////////////////////////////////////////


template<>
NFE_INLINE size_t BitUtils<uint64>::CountTrailingZeros(const uint64 x)
{
#if defined(_M_X64) || defined(__amd64__)
    return __builtin_ctzll(x);
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
    return __builtin_ctzl(x);
}

template<>
NFE_INLINE size_t BitUtils<uint16>::CountTrailingZeros(const uint16 x)
{
    return __builtin_ctzl(x);
}

template<>
NFE_INLINE size_t BitUtils<uint8>::CountTrailingZeros(const uint8 x)
{
    return __builtin_ctzl(x);
}


//////////////////////////////////////////////////////////////////////////


template<>
NFE_INLINE uint16 BitUtils<uint16>::SwapEndianness(uint16 x)
{
    return __builtin_bswap16(x);
}

template<>
NFE_INLINE uint32 BitUtils<uint32>::SwapEndianness(uint32 x)
{
    return __builtin_bswap32(x);
}

template<>
NFE_INLINE uint64 BitUtils<uint64>::SwapEndianness(uint64 x)
{
    return __builtin_bswap64(x);
}

} // namespace Common
} // namespace NFE