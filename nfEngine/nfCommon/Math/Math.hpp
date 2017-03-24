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
        #error "SSE4 must be enabled when using AVX"
    #endif // NFE_USE_SSE4
#include <immintrin.h>
#endif // defined(NFE_USE_AVX) | defined(NFE_USE_FMA)


#define NFE_MATH_EPSILON (0.000001f)
#define NFE_MATH_PI (3.14159265359f)
#define NFE_MATH_2PI (6.28318530718f)
#define NFE_MATH_E (2.7182818f)


namespace NFE {
namespace Math {

/**
 * Structure for efficient 2D vector storing.
 */
struct Float2
{
    float x, y;

    Float2() : x(0.0f), y(0.0f) {};
    Float2(float x_, float y_) : x(x_), y(y_) {};
    Float2(float* pArr) : x(pArr[0]), y(pArr[1]) {};
};

/**
 * Structure for efficient 3D vector storing.
 */
struct Float3
{
    float x, y, z;

    Float3() : x(0.0f), y(0.0f), z(0.0f) {};
    Float3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {};
    Float3(float* pArr) : x(pArr[0]), y(pArr[1]), z(pArr[2]) {};
};

/**
 * Structure for efficient 4D vector storing - unaligned version of Vector class.
 */
struct Float4
{
    float x, y, z, w;

    Float4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {};
    Float4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {};
    Float4(float* pArr) : x(pArr[0]), y(pArr[1]), z(pArr[2]), w(pArr[3]) {};
};


/**
 * Rectangle template.
 */
template<typename T>
struct Rect
{
    T Xmin, Xmax, Ymin, Ymax;

    Rect() : Xmin(0), Xmax(0), Ymin(0), Ymax(0) { }

    Rect(T Xmin_, T Ymin_, T Xmax_, T Ymax_)
        : Xmin(Xmin_), Xmax(Xmax_), Ymin(Ymin_), Ymax(Ymax_) { }
};

typedef Rect<int> Recti;
typedef Rect<float> Rectf;


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

/**
 * Minimum.
 */
template<typename T>
NFE_INLINE T Min(const T a, const T b)
{
    return (a < b) ? a : b;
}

/**
 * Maximum.
 */
template<typename T>
NFE_INLINE T Max(const T a, const T b)
{
    return (a < b) ? b : a;
}

/**
 * Absolute value.
 */
template<typename T>
NFE_INLINE T Abs(const T x)
{
    if (x < static_cast<T>(0))
        return -x;

    return x;
}

/**
 * Returns x with sign of y
 */
NFE_INLINE float CopySignF(const float x, const float y)
{
    Bits32 xInt, yInt;
    xInt.f = x;
    yInt.f = y;
    xInt.ui = (0x7fffffff & xInt.ui) | (0x80000000 & yInt.ui);
    return xInt.f;
}

/**
 * Clamp to range.
 */
template<typename T>
NFE_INLINE T Clamp(const T x, const T min, const T max)
{
    if (x > max)
        return max;
    else if (x < min)
        return min;
    else
        return x;
}

/**
 * Linear interpolation.
 */
template<typename T>
NFE_INLINE T Lerp(const T a, const T b, const T w)
{
    return a + w * (b - a);
}

/**
 * Rounds down "x" to nearest multiply of "step"
 */
NFE_INLINE float Quantize(float x, float step)
{
    float tmp = x / step;
    tmp = floorf(tmp);
    return tmp * step;
}

/**
 * Check if a given number is NaN (not a number), according to IEEE 754 standard.
 */
NFE_INLINE bool IsNaN(float a)
{
    Bits32 num;
    num.f = a;
    return ((num.ui & 0x7F800000) == 0x7F800000) && ((num.ui & 0x7FFFFF) != 0);
}

/**
 * Check if a given number is infinity (positive or negative), according to IEEE 754 standard.
 */
NFE_INLINE bool IsInfinity(float a)
{
    Bits32 num;
    num.f = a;
    return (num.ui & 0x7FFFFFFF) == 0x7F800000;
}

/**
 * Wang hash.
 */
NFE_INLINE unsigned int Hash(unsigned int x)
{
    x = (x ^ 61) ^ (x >> 16);
    x *= 9;
    x = x ^ (x >> 4);
    x *= 0x27d4eb2d;
    x = x ^ (x >> 15);
    return x;
}

/**
 * Check if a number is power of two.
 */
template<typename T>
NFE_INLINE constexpr bool PowerOfTwo(const T x)
{
    return x && !(x & (x - 1));
}

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
