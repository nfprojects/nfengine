#pragma once

#include "Math.hpp"
#include "Vector4.hpp"
#include "VectorBool8.hpp"

namespace NFE {
namespace Math {

/**
 * 8-element SIMD vector.
 */
struct NFE_ALIGN(32) Vector8
{
    // constructors
    NFE_FORCE_INLINE Vector8();
    NFE_FORCE_INLINE Vector8(const Vector8& other);
    NFE_FORCE_INLINE static const Vector8 Zero();
    NFE_FORCE_INLINE explicit Vector8(const Vector4& lo);
    NFE_FORCE_INLINE Vector8(const Vector4& lo, const Vector4& hi);
    NFE_FORCE_INLINE explicit Vector8(const float scalar);
    NFE_FORCE_INLINE explicit Vector8(const int32 scalar);
    NFE_FORCE_INLINE explicit Vector8(const uint32 scalar);
    NFE_FORCE_INLINE Vector8(float e0, float e1, float e2, float e3, float e4, float e5, float e6, float e7);
    NFE_FORCE_INLINE Vector8(int32 e0, int32 e1, int32 e2, int32 e3, int32 e4, int32 e5, int32 e6, int32 e7);
    NFE_FORCE_INLINE Vector8(uint32 e0, uint32 e1, uint32 e2, uint32 e3, uint32 e4, uint32 e5, uint32 e6, uint32 e7);
    NFE_FORCE_INLINE Vector8(const float* src);
    NFE_FORCE_INLINE Vector8& operator = (const Vector8& other);
    NFE_FORCE_INLINE static const Vector8 FromInteger(int32 x);

    // Rearrange vector elements (in both lanes, parallel)
    template<uint32 ix = 0, uint32 iy = 1, uint32 iz = 2, uint32 iw = 3>
    NFE_FORCE_INLINE const Vector8 Swizzle() const;

#ifdef NFE_USE_AVX
    NFE_FORCE_INLINE Vector8(const __m256 & m);
    NFE_FORCE_INLINE operator __m256() const { return v; }
    NFE_FORCE_INLINE operator __m256i() const { return reinterpret_cast<const __m256i*>(&v)[0]; }
#endif // NFE_USE_AVX

    NFE_FORCE_INLINE float operator[] (uint32 index) const { return f[index]; }
    NFE_FORCE_INLINE float& operator[] (uint32 index) { return f[index]; }

    // extract lower lanes
    NFE_FORCE_INLINE const Vector4 Low() const;
    // extract higher lanes
    NFE_FORCE_INLINE const Vector4 High() const;

    // simple arithmetics
    NFE_FORCE_INLINE const Vector8 operator - () const;
    NFE_FORCE_INLINE const Vector8 operator + (const Vector8& b) const;
    NFE_FORCE_INLINE const Vector8 operator - (const Vector8& b) const;
    NFE_FORCE_INLINE const Vector8 operator * (const Vector8& b) const;
    NFE_FORCE_INLINE const Vector8 operator / (const Vector8& b) const;
    NFE_FORCE_INLINE const Vector8 operator * (float b) const;
    NFE_FORCE_INLINE const Vector8 operator / (float b) const;
    NFE_FORCE_INLINE Vector8& operator += (const Vector8& b);
    NFE_FORCE_INLINE Vector8& operator -= (const Vector8& b);
    NFE_FORCE_INLINE Vector8& operator *= (const Vector8& b);
    NFE_FORCE_INLINE Vector8& operator /= (const Vector8& b);
    NFE_FORCE_INLINE Vector8& operator *= (float b);
    NFE_FORCE_INLINE Vector8& operator /= (float b);
    friend const Vector8 operator * (float a, const Vector8& b);

    // comparison operators (returns true, if all the elements satisfy the equation)
    NFE_FORCE_INLINE const VectorBool8 operator == (const Vector8& b) const;
    NFE_FORCE_INLINE const VectorBool8 operator < (const Vector8& b) const;
    NFE_FORCE_INLINE const VectorBool8 operator <= (const Vector8& b) const;
    NFE_FORCE_INLINE const VectorBool8 operator > (const Vector8& b) const;
    NFE_FORCE_INLINE const VectorBool8 operator >= (const Vector8& b) const;
    NFE_FORCE_INLINE const VectorBool8 operator != (const Vector8& b) const;

    // bitwise logic operations
    NFE_FORCE_INLINE const Vector8 operator & (const Vector8& b) const;
    NFE_FORCE_INLINE const Vector8 operator | (const Vector8& b) const;
    NFE_FORCE_INLINE const Vector8 operator ^ (const Vector8& b) const;
    NFE_FORCE_INLINE Vector8& operator &= (const Vector8& b);
    NFE_FORCE_INLINE Vector8& operator |= (const Vector8& b);
    NFE_FORCE_INLINE Vector8& operator ^= (const Vector8& b);

    NFE_FORCE_INLINE static const Vector8 Floor(const Vector8& v);
    NFE_FORCE_INLINE static const Vector8 Sqrt(const Vector8& v);
    NFE_FORCE_INLINE static const Vector8 Reciprocal(const Vector8& v);
    NFE_FORCE_INLINE static const Vector8 FastReciprocal(const Vector8& v);
    NFE_FORCE_INLINE static const Vector8 Lerp(const Vector8& v1, const Vector8& v2, const Vector8& weight);
    NFE_FORCE_INLINE static const Vector8 Lerp(const Vector8& v1, const Vector8& v2, float weight);
    NFE_FORCE_INLINE static const Vector8 Min(const Vector8& a, const Vector8& b);
    NFE_FORCE_INLINE static const Vector8 Max(const Vector8& a, const Vector8& b);
    NFE_FORCE_INLINE static const Vector8 Abs(const Vector8& v);
    NFE_FORCE_INLINE const Vector8 Clamped(const Vector8& min, const Vector8& max) const;

    // Build mask of sign bits.
    NFE_FORCE_INLINE uint32 GetSignMask() const;

    // For each vector component, copy value from "a" if "sel" is "false", or from "b" otherwise.
    NFE_FORCE_INLINE static const Vector8 Select(const Vector8& a, const Vector8& b, const VectorBool8& sel);

    template<uint32 selX, uint32 selY, uint32 selZ, uint32 selW>
    NFE_FORCE_INLINE static const Vector8 Select(const Vector8& a, const Vector8& b);

    // Check if the vector is equal to zero
    NFE_FORCE_INLINE bool IsZero() const;

    // Check if any component is NaN
    NFE_FORCE_INLINE bool IsNaN() const;

    // Check if any component is an infinity
    NFE_FORCE_INLINE bool IsInfinite() const;

    // Check if is not NaN or infinity
    NFE_FORCE_INLINE bool IsValid() const;

    // Check if two vectors are (almost) equal.
    NFE_FORCE_INLINE static bool AlmostEqual(const Vector8& v1, const Vector8& v2, float epsilon = NFE_MATH_EPSILON);

    // Fused multiply and add (a * b + c)
    NFE_FORCE_INLINE static const Vector8 MulAndAdd(const Vector8& a, const Vector8& b, const Vector8& c);
    NFE_FORCE_INLINE static const Vector8 MulAndAdd(const Vector8& a, const float b, const Vector8& c);

    // Fused multiply and subtract (a * b - c)
    NFE_FORCE_INLINE static const Vector8 MulAndSub(const Vector8& a, const Vector8& b, const Vector8& c);
    NFE_FORCE_INLINE static const Vector8 MulAndSub(const Vector8& a, const float b, const Vector8& c);

    // Fused multiply (negated) and add (a * b + c)
    // Fused multiply (negated) and add (a * b + c)
    NFE_FORCE_INLINE static const Vector8 NegMulAndAdd(const Vector8& a, const Vector8& b, const Vector8& c);
    NFE_FORCE_INLINE static const Vector8 NegMulAndAdd(const Vector8& a, const float b, const Vector8& c);

    // Fused multiply (negated) and subtract (a * b - c)
    NFE_FORCE_INLINE static const Vector8 NegMulAndSub(const Vector8& a, const Vector8& b, const Vector8& c);
    NFE_FORCE_INLINE static const Vector8 NegMulAndSub(const Vector8& a, const float b, const Vector8& c);

    // Calculate horizontal maximum. Result is splatted across all elements.
    NFE_FORCE_INLINE const Vector8 HorizontalMax() const;

    // Compute fmodf(x, 1.0f)
    NFE_FORCE_INLINE static const Vector8 Fmod1(const Vector8& x);

    // transpose 8x8 matrix
    //NFE_FORCE_INLINE static void Transpose8x8(Vector8& v0, Vector8& v1, Vector8& v2, Vector8& v3, Vector8& v4, Vector8& v5, Vector8& v6, Vector8& v7);

private:

    friend struct VectorInt8;

    union
    {
        float f[8];
        int32 i[8];
        uint32 u[8];
#ifdef NFE_USE_AVX
        __m256 v;
#else
        struct
        {
            Vector4 low;
            Vector4 high;
        };
#endif // NFE_USE_AVX
    };
};

// like Vector8::operator * (float)
NFE_FORCE_INLINE const Vector8 operator*(float a, const Vector8& b);

// some commonly used constants
NFE_GLOBAL_CONST Vector8 VECTOR8_EPSILON = { NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON };
NFE_GLOBAL_CONST Vector8 VECTOR8_HALVES = { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
NFE_GLOBAL_CONST Vector8 VECTOR8_MIN = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };
NFE_GLOBAL_CONST Vector8 VECTOR8_MAX = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
NFE_GLOBAL_CONST Vector8 VECTOR8_INF = { std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity() };
NFE_GLOBAL_CONST Vector8 VECTOR8_ONE = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
NFE_GLOBAL_CONST Vector8 VECTOR8_MINUS_ONE = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f };
NFE_GLOBAL_CONST Vector8 VECTOR8_MASK_X = { 0xFFFFFFFFu, 0u, 0u, 0u, 0u, 0u, 0u, 0u };
NFE_GLOBAL_CONST Vector8 VECTOR8_MASK_Y = { 0u, 0xFFFFFFFFu, 0u, 0u, 0u, 0u, 0u, 0u };
NFE_GLOBAL_CONST Vector8 VECTOR8_MASK_Z = { 0u, 0u, 0xFFFFFFFF, 0u, 0u, 0u, 0u, 0u };
NFE_GLOBAL_CONST Vector8 VECTOR8_MASK_W = { 0u, 0u, 0u, 0xFFFFFFFF, 0u, 0u, 0u, 0u };
NFE_GLOBAL_CONST Vector8 VECTOR8_MASK_ALL = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
NFE_GLOBAL_CONST Vector8 VECTOR8_MASK_ABS = { 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF };
NFE_GLOBAL_CONST Vector8 VECTOR8_MASK_SIGN = { 0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u };
NFE_GLOBAL_CONST Vector8 VECTOR8_INV_255 = { 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f };
NFE_GLOBAL_CONST Vector8 VECTOR8_255 = { 255.0f, 255.0f, 255.0f, 255.0f, 255.0f, 255.0f, 255.0f, 255.0f };

} // namespace Math
} // namespace NFE

#ifdef NFE_USE_AVX
#include "Vector8ImplAVX.hpp"
#else
#include "Vector8ImplNaive.hpp"
#endif // NFE_USE_AVX

#include "Vector8ImplCommon.hpp"
