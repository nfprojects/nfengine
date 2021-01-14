#pragma once

#include "Vec16f.hpp"

namespace NFE {
namespace Math {

/**
 * Accurate sine and cosine.
 * @note    This is faster than sinf/cosf
 * @note    Maximum absolute error: about 5.0e-07
 */
NFCOMMON_API float Sin(float x);
NFCOMMON_API const Vec4f Sin(const Vec4f& x);
NFCOMMON_API const Vec8f Sin(const Vec8f& x);
NFCOMMON_API const Vec16f Sin(const Vec16f& x);

NFE_FORCE_INLINE float Cos(float x);
NFE_FORCE_INLINE const Vec4f Cos(const Vec4f& x);
NFE_FORCE_INLINE const Vec8f Cos(const Vec8f& x);

// Compute sine and cosine in one go
NFE_FORCE_INLINE const Vec4f SinCos(const float x);

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

// Maximum absolute error: about 7.0e-5
NFCOMMON_API float FastACos(float x);

/**
 * Fast atan2f.
 * Max relative error ~ 3.6e-5
 */
float FastATan2(const float y, const float x);

/**
 * Fast exponent.
 * @note    This is much faster than expf
 * @note    Maximum relative error: about 0.2%
 */
NFE_FORCE_INLINE float FastExp(float x);
NFE_FORCE_INLINE const Vec4f FastExp(const Vec4f& x);
NFE_FORCE_INLINE const Vec8f FastExp(const Vec8f& x);

/**
 * Fast base-2 exponent.
 * @note    This is much faster than exp2f
 * @note    Maximum relative error: about 0.2%
 */
NFCOMMON_API float FastExp2(float x);
NFCOMMON_API const Vec4f FastExp2(const Vec4f& x);
NFCOMMON_API const Vec8f FastExp2(const Vec8f& x);

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
NFCOMMON_API const Vec4f FastLog(const Vec4f& x);

/**
 * Fast base-2 logarithm.
 * @note    This is faster log2f
 */
NFCOMMON_API float FastLog2(float x);
NFCOMMON_API const Vec4f FastLog2(const Vec4f& x);

/**
 * Error Function and its inverse
 */
NFCOMMON_API float Erf(float x);
NFCOMMON_API float ErfInv(float x);

} // namespace Math
} // namespace NFE


#include "TranscendentalImpl.hpp"