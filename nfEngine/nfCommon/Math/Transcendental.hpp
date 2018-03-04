/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of transcendental functions.
 */

#pragma once

#include "../nfCommon.hpp"


namespace NFE {
namespace Math {


// TODO atan2
// TODO fast sin/cos


/**
 * Accurate sine and cosine.
 * @note    This is faster than sinf/cosf
 * @note    Maximum absolute error: about 5.0e-07
 */
NFCOMMON_API float Sin(float x);
NFCOMMON_API float Cos(float x);

/**
 * Accurate tangent and cotangent.
 * @note    This is slow.
 */
NFE_INLINE float Tan(float x);
NFE_INLINE float Cot(float x);

/**
 * Accurate inverse trigonometric functions.
 * @note    This is faster than asinf/acosf/atanf
 * @note    Maximum absolute error: about 2.0e-07
 */
NFCOMMON_API float ASin(float x);
NFCOMMON_API float ACos(float x);
NFCOMMON_API float ATan(float x);

/**
 * Fast exponent.
 * @note    This is much faster than expf
 * @note    Maximum relative error: about 0.2%
 */
NFCOMMON_API float FastExp(float x);

/**
 * Accurate natural logarithm.
 * @note    Maximum relative error: about 0.01%
 */
NFCOMMON_API float Log(float x);

/**
 * Fast natural logarithm.
 * @note    This is faster logf
 * @note    Maximum relative error: about 0.07%
 */
NFCOMMON_API float FastLog(float x);


} // namespace Math
} // namespace NFE


#include "TranscendentalImpl.hpp"