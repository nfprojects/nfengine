/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Basic math classes and functions declarations.
 */

#pragma once

#include "../nfCommon.hpp"
#include <math.h>

// TODO move these below to PCH as well

#ifdef NFE_USE_SSE
#include <xmmintrin.h>
#endif // NFE_USE_SSE

#ifdef NFE_USE_SSE4
    #ifndef NFE_USE_SSE
        #error "SSE must be enabled when using SSE4"
    #endif // NFE_USE_SSE
#include <smmintrin.h>
#endif // NFE_USE_SSE4

#if defined(NFE_USE_AVX) | defined(NFE_USE_FMA)
    #ifndef NFE_USE_SSE4
        #error "SSE4 must be enabled when using AVX or FMA"
    #endif // NFE_USE_SSE4
#include <immintrin.h>
#endif // defined(NFE_USE_AVX) | defined(NFE_USE_FMA)

#if defined(NFE_USE_AVX2)
    #ifndef NFE_USE_AVX
        #error "AVX must be enabled when using AVX2"
    #endif // NFE_USE_AVX
#endif // defined(NFE_USE_AVX2)


#define NFE_MATH_EPSILON (0.000001f)
#define NFE_MATH_PI (3.14159265359f)
#define NFE_MATH_2PI (6.28318530718f)
#define NFE_MATH_E (2.7182818f)


namespace NFE {
namespace Math {



/**
 * Union providing easy manipulations on 32-bit values;
 */
union Bits32
{
    float f;
    uint32 ui;
    int32 si;
};

/**
 * Union providing easy manipulations on 64-bit values;
 */
union Bits64
{
    double f;
    uint64 ui;
    int64 si;
};


//////////////////////////////////////////////////////////////////////////


/**
 * Minimum of a single value.
 * @note    This is required for the Min() variadic template function to work.
 */
template<typename T, typename ... Types>
NFE_INLINE constexpr T Min(const T& a);

/**
 * Minimum of multiple values.
 */
template<typename T, typename ... Types>
NFE_INLINE constexpr T Min(const T& a, const T& b, const Types& ... r);

/**
 * Maximum of a single value.
 * @note    This is required for the Max() variadic template function to work.
 */
template<typename T, typename ... Types>
NFE_INLINE constexpr T Max(const T& a);

/**
 * Maximum of multiple values.
 */
template<typename T, typename ... Types>
NFE_INLINE constexpr T Max(const T& a, const T& b, const Types& ... r);

/**
 * Absolute value.
 */
template<typename T>
NFE_INLINE constexpr T Abs(const T& x);

/**
 * Returns 'x' with a sign of 'y'.
 */
template<typename T>
NFE_INLINE T CopySign(T x, T y);

/**
 * Clamp to range.
 */
template<typename T>
NFE_INLINE T Clamp(const T x, const T min, const T max);

/**
 * Linear interpolation.
 */
template<typename T>
NFE_INLINE constexpr T Lerp(const T& a, const T& b, const T w);

/**
 * Rounds down "x" to nearest multiply of "step"
 */
NFE_INLINE float Quantize(float x, float step);

/**
 * Check if a given number is NaN (not a number), according to IEEE 754 standard.
 */
template<typename T>
NFE_INLINE bool IsNaN(T a);

/**
 * Check if a given number is infinity (positive or negative), according to IEEE 754 standard.
 */
template<typename T>
NFE_INLINE bool IsInfinity(T a);

/**
 * Wang hash.
 */
NFE_INLINE unsigned int Hash(unsigned int x);

/**
 * Check if a number is power of two.
 */
template<typename T>
NFE_INLINE constexpr bool IsPowerOfTwo(const T x);

/**
 * Convert radians to degrees.
 */
NFE_INLINE constexpr float RadToDeg(const float radians);

/**
 * Convert degrees to radians.
 */
NFE_INLINE constexpr float DegToRad(const float degrees);

//////////////////////////////////////////////////////////////////////////


/**
 * Solve a system of linear equations with 3 variables.
 *
 * @param M     Coefficients matrix.
 * @param x,y,z Results.
 *
 * @return True on success (the system has exactly one solution).
 */
NFCOMMON_API bool SolveEquationsSystem3(float M[4][3], float& x, float& y, float& z);

} // namespace Math
} // namespace NFE


// Inline definitions go there:
#include "MathImpl.hpp"
