#pragma once

#include "Vector4.hpp"
#include "Vector8.hpp"

namespace NFE {
namespace Math {

/**
 * Accurate sine and cosine.
 * @note    This is faster than sinf/cosf
 * @note    Maximum absolute error: about 5.0e-07
 */
NFCOMMON_API NFE_FORCE_NOINLINE float Sin(float x);
NFCOMMON_API NFE_FORCE_NOINLINE const Vector4 Sin(const Vector4& x);
NFCOMMON_API NFE_FORCE_NOINLINE const Vector8 Sin(const Vector8& x);

NFE_FORCE_INLINE float Cos(float x);
NFE_FORCE_INLINE const Vector4 Cos(const Vector4& x);
NFE_FORCE_INLINE const Vector8 Cos(const Vector8& x);

// Compute sine and cosine in one go
NFE_FORCE_INLINE const Vector4 SinCos(const float x);

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
NFCOMMON_API float FastExp(float x);
NFCOMMON_API const Vector4 FastExp(const Vector4& x);
NFCOMMON_API const Vector8 FastExp(const Vector8& x);

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
NFCOMMON_API const Vector4 FastLog(const Vector4& x);

} // namespace Math
} // namespace NFE


#include "TranscendentalImpl.hpp"