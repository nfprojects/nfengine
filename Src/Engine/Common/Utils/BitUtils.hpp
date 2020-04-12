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
 * Class providing bit-manipulation methods.
 */
template<typename T>
class BitUtils
{
public:
    /**
     * Count number of bits set to '1'.
     */
    static uint32 CountBits(T x);

    /**
     * Count leading (most significant) zeros.
     */
    static uint32 CountLeadingZeros(T x);

    /**
     * Count trailing (least significant) zeros.
     */
    static uint32 CountTrailingZeros(T x);

    /**
     * Reverse bits order.
     */
    static T ReverseBits(T x);

    /**
     * Swap bytes order.
     */
    static T SwapEndianness(T x);

    /**
     * Creates bit mask of given length.
     */
    static T CreateBitMask(uint32 length);
};

} // namespace Common
} // namespace NFE


#include "BitUtilsImplCommon.hpp"

#ifdef NFE_USE_AVX
#ifdef WIN32
#include "BitUtilsImplWindowsAVX.hpp"
#else // WIN32
#include "BitUtilsImplLinuxAVX.hpp"
#endif //WIN32
#else // NFE_USE_AVX
#include "BitUtilsImplNative.hpp"
#endif // NFE_USE_AVX