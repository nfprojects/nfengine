/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Inline definitions of basic math functions.
 */

#pragma once

#include "Math.hpp"


namespace NFE {
namespace Math {


template<typename T, typename ... Types>
NFE_INLINE constexpr T Min(T a)
{
    return a;
}

template<typename T, typename ... Types>
NFE_INLINE constexpr T Min(T a, T b, Types ... r)
{
    return Min(a < b ? a : b, r ...);
}

template<typename T, typename ... Types>
NFE_INLINE constexpr T Max(T a)
{
    return a;
}

template<typename T, typename ... Types>
NFE_INLINE constexpr T Max(T a, T b, Types ... r)
{
    return Max(a > b ? a : b, r ...);
}

template<typename T>
NFE_INLINE constexpr T Median(T a, T b, T c)
{
    return (a > b) ? Max(b, Min(a, c)) : Min(b, Max(a, c));
}

template<typename T>
NFE_INLINE constexpr T Abs(T x)
{
    return (x < static_cast<T>(0)) ? (-x) : x;
}

template<>
NFE_INLINE float CopySign(float x, float y)
{
    Bits32 xInt, yInt;
    xInt.f = x;
    yInt.f = y;
    xInt.ui = (0x7fffffff & xInt.ui) | (0x80000000 & yInt.ui);
    return xInt.f;
}

template<>
NFE_INLINE double CopySign(double x, double y)
{
    Bits64 xInt, yInt;
    xInt.f = x;
    yInt.f = y;
    xInt.ui = (0x7fffffffffffffffull & xInt.ui) | (0x8000000000000000ull & yInt.ui);
    return xInt.f;
}

template<typename T>
NFE_INLINE constexpr T Clamp(T x, T min, T max)
{
    return (x > max) ? max : ((x < min) ? min : x);
}

template<typename T>
NFE_INLINE constexpr T Lerp(T a, T b, T weight)
{
    return a + w * (b - a);
}

NFE_INLINE float Quantize(float x, float step)
{
    float tmp = x / step;
    tmp = floorf(tmp);
    return tmp * step;
}

template<>
NFE_INLINE bool IsNaN(float a)
{
    Bits32 num;
    num.f = a;
    return ((num.ui & 0x7F800000) == 0x7F800000) && ((num.ui & 0x7FFFFF) != 0);
}

template<>
NFE_INLINE bool IsInfinity(float a)
{
    Bits32 num;
    num.f = a;
    return (num.ui & 0x7FFFFFFF) == 0x7F800000;
}

NFE_INLINE unsigned int Hash(unsigned int x)
{
    x = (x ^ 61) ^ (x >> 16);
    x *= 9;
    x = x ^ (x >> 4);
    x *= 0x27d4eb2d;
    x = x ^ (x >> 15);
    return x;
}

template<typename T>
NFE_INLINE constexpr bool IsPowerOfTwo(const T x)
{
    return x && !(x & (x - 1));
}

NFE_INLINE constexpr float RadToDeg(const float radians)
{
    return radians * 57.2957795f;
}

NFE_INLINE constexpr float DegToRad(const float degrees)
{
    return degrees * 0.0174532925f;
}

} // namespace Math
} // namespace NFE
