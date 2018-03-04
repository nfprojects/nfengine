/**
 * @file
 * @brief  Declarations of transcendental functions.
 */

#pragma once

#include "Math.hpp"


namespace NFE {
namespace Math {


/**
 * Accurate sine and cosine.
 * Maximum absolute error: 5.0e-07 for float, 3.0e-08 for double.
 */
NFCOMMON_API float Sin(float x);
NFCOMMON_API float Cos(float x);

/**
 * Accurate tangent and cotangent.
 * Note: this is slow.
 */
NFE_INLINE float Tan(float x);
NFE_INLINE float Cot(float x);

/**
 * Accurate inverse trigonometric functions.
 */
NFCOMMON_API float ASin(float x);
NFCOMMON_API float ACos(float x);
NFCOMMON_API float ATan(float x);

/**
 * Exponent.
 */
NFCOMMON_API float Exp(float x);

/**
 * Fast exponent.
 * Maximum relative error: about 0.2%
 */
NFCOMMON_API float FastExp(float x);

/**
 * Natural logarithm
 */
NFCOMMON_API float Log(float x);

/**
 * Fast natural logarithm
 */
NFCOMMON_API float FastLog(float x);


} // namespace Math
} // namespace NFE


#include "TranscendentalImpl.hpp"