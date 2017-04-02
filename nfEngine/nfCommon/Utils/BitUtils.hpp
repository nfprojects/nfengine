/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @author Witek902 (witek902@gmail.com)
 * @brief  File with defined bitwise operations
 */
#pragma once

#include "../nfCommon.hpp"


namespace NFE {
namespace Common {

/**
 * Count number of bits set to '1'.
 */
template<typename T>
size_t CountBits(T x);

/**
 * Count leading (most significant) zeros.
 */
template<typename T>
size_t CountLeadingZeros(T x);

/**
 * Count trailing (least significant) zeros.
 */
template<typename T>
size_t CountTrailingZeros(T x);

/**
 * Reverse bits order.
 */
template<typename T>
T ReverseBits(T x);

/**
 * Swap bytes order.
 */
template<typename T>
T SwapEndianness(T x);

/**
 * Creates bit mask of given length.
 */
NFE_INLINE uint32 CreateBitMask(uint8 length);


} // namespace Common
} // namespace NFE

#include "BitUtilsImpl.hpp"
