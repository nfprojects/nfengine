#pragma once

#include "Math.hpp"
#include "Vec4f.hpp"

namespace NFE {
namespace Math {

/**
 * Boolean vector for Vec8f type.
 */
struct NFE_ALIGN(32) VecBool8f : public Common::Aligned<32>
{
    VecBool8f() = default;

    NFE_FORCE_INLINE explicit VecBool8f(bool e0, bool e1, bool e2, bool e3, bool e4, bool e5, bool e6, bool e7);

#ifdef NFE_USE_AVX
    NFE_FORCE_INLINE VecBool8f(const __m256 other) : v(other) { }
    NFE_FORCE_INLINE VecBool8f(const __m256i other) : v(_mm256_castsi256_ps(other)) { }
    NFE_FORCE_INLINE operator __m256() const { return v; }
    NFE_FORCE_INLINE operator __m256i() const { return _mm256_castps_si256(v); }
#endif // NFE_USE_AVX

    template<uint32 index>
    NFE_FORCE_INLINE bool Get() const;

    // combine into 8-bit mask
    NFE_FORCE_INLINE int32 GetMask() const;

    NFE_FORCE_INLINE bool All() const;
    NFE_FORCE_INLINE bool None() const;
    NFE_FORCE_INLINE bool Any() const;

    NFE_FORCE_INLINE const VecBool8f operator & (const VecBool8f rhs) const;
    NFE_FORCE_INLINE const VecBool8f operator | (const VecBool8f rhs) const;
    NFE_FORCE_INLINE const VecBool8f operator ^ (const VecBool8f rhs) const;

    NFE_FORCE_INLINE bool operator == (const VecBool8f rhs) const;

private:
    friend struct Vec8f;
    friend struct VecBool8i;

#ifdef NFE_USE_AVX
    __m256 v;
#else
    bool b[8];
#endif // NFE_USE_AVX
};

/**
 * 8-element SIMD vector.
 */
struct NFE_ALIGN(32) Vec8f : public Common::Aligned<32>
{
    using BoolType = VecBool8f;

    // constructors
    NFE_FORCE_INLINE Vec8f();
    NFE_FORCE_INLINE Vec8f(const Vec8f& other);
    NFE_FORCE_INLINE static const Vec8f Zero();
    NFE_FORCE_INLINE explicit Vec8f(const Vec4f& lo);
    NFE_FORCE_INLINE Vec8f(const Vec4f& lo, const Vec4f& hi);
    NFE_FORCE_INLINE explicit Vec8f(const float scalar);
    NFE_FORCE_INLINE explicit Vec8f(const int32 scalar);
    NFE_FORCE_INLINE explicit Vec8f(const uint32 scalar);
    NFE_FORCE_INLINE Vec8f(float e0, float e1, float e2, float e3, float e4, float e5, float e6, float e7);
    NFE_FORCE_INLINE Vec8f(int32 e0, int32 e1, int32 e2, int32 e3, int32 e4, int32 e5, int32 e6, int32 e7);
    NFE_FORCE_INLINE Vec8f(uint32 e0, uint32 e1, uint32 e2, uint32 e3, uint32 e4, uint32 e5, uint32 e6, uint32 e7);
    NFE_FORCE_INLINE Vec8f(const float* src);
    NFE_FORCE_INLINE Vec8f& operator = (const Vec8f& other);
    NFE_FORCE_INLINE static const Vec8f FromInteger(int32 x);

    // Rearrange vector elements (in both lanes, parallel)
    template<uint32 ix = 0, uint32 iy = 1, uint32 iz = 2, uint32 iw = 3>
    NFE_FORCE_INLINE const Vec8f Swizzle() const;

#ifdef NFE_USE_AVX
    NFE_FORCE_INLINE Vec8f(const __m256 & m);
    NFE_FORCE_INLINE operator __m256() const { return v; }
    NFE_FORCE_INLINE operator __m256i() const { return reinterpret_cast<const __m256i*>(&v)[0]; }
#endif // NFE_USE_AVX

    NFE_FORCE_INLINE float operator[] (uint32 index) const
    {
        NFE_ASSERT(index < 8, "Index out of bounds (%u)", index);
        return f[index];
    }

    NFE_FORCE_INLINE float& operator[] (uint32 index)
    {
        NFE_ASSERT(index < 8, "Index out of bounds (%u)", index);
        return f[index];
    }

    // extract lower lanes
    NFE_FORCE_INLINE const Vec4f Low() const;
    // extract higher lanes
    NFE_FORCE_INLINE const Vec4f High() const;

    // simple arithmetics
    NFE_FORCE_INLINE const Vec8f operator - () const;
    NFE_FORCE_INLINE const Vec8f operator + (const Vec8f& b) const;
    NFE_FORCE_INLINE const Vec8f operator - (const Vec8f& b) const;
    NFE_FORCE_INLINE const Vec8f operator * (const Vec8f& b) const;
    NFE_FORCE_INLINE const Vec8f operator / (const Vec8f& b) const;
    NFE_FORCE_INLINE const Vec8f operator * (float b) const;
    NFE_FORCE_INLINE const Vec8f operator / (float b) const;
    NFE_FORCE_INLINE Vec8f& operator += (const Vec8f& b);
    NFE_FORCE_INLINE Vec8f& operator -= (const Vec8f& b);
    NFE_FORCE_INLINE Vec8f& operator *= (const Vec8f& b);
    NFE_FORCE_INLINE Vec8f& operator /= (const Vec8f& b);
    NFE_FORCE_INLINE Vec8f& operator *= (float b);
    NFE_FORCE_INLINE Vec8f& operator /= (float b);
    friend const Vec8f operator * (float a, const Vec8f& b);

    // comparison operators (returns true, if all the elements satisfy the equation)
    NFE_FORCE_INLINE const BoolType operator == (const Vec8f& b) const;
    NFE_FORCE_INLINE const BoolType operator < (const Vec8f& b) const;
    NFE_FORCE_INLINE const BoolType operator <= (const Vec8f& b) const;
    NFE_FORCE_INLINE const BoolType operator > (const Vec8f& b) const;
    NFE_FORCE_INLINE const BoolType operator >= (const Vec8f& b) const;
    NFE_FORCE_INLINE const BoolType operator != (const Vec8f& b) const;

    // bitwise logic operations
    NFE_FORCE_INLINE const Vec8f operator & (const Vec8f& b) const;
    NFE_FORCE_INLINE const Vec8f operator | (const Vec8f& b) const;
    NFE_FORCE_INLINE const Vec8f operator ^ (const Vec8f& b) const;
    NFE_FORCE_INLINE Vec8f& operator &= (const Vec8f& b);
    NFE_FORCE_INLINE Vec8f& operator |= (const Vec8f& b);
    NFE_FORCE_INLINE Vec8f& operator ^= (const Vec8f& b);

    NFE_FORCE_INLINE static const Vec8f Floor(const Vec8f& v);
    NFE_FORCE_INLINE static const Vec8f Sqrt(const Vec8f& v);
    NFE_FORCE_INLINE static const Vec8f Reciprocal(const Vec8f& v);
    NFE_FORCE_INLINE static const Vec8f FastReciprocal(const Vec8f& v);
    NFE_FORCE_INLINE static const Vec8f Lerp(const Vec8f& v1, const Vec8f& v2, const Vec8f& weight);
    NFE_FORCE_INLINE static const Vec8f Lerp(const Vec8f& v1, const Vec8f& v2, float weight);
    NFE_FORCE_INLINE static const Vec8f Min(const Vec8f& a, const Vec8f& b);
    NFE_FORCE_INLINE static const Vec8f Max(const Vec8f& a, const Vec8f& b);
    NFE_FORCE_INLINE static const Vec8f Abs(const Vec8f& v);
    NFE_FORCE_INLINE const Vec8f Clamped(const Vec8f& min, const Vec8f& max) const;

    // Build mask of sign bits.
    NFE_FORCE_INLINE uint32 GetSignMask() const;

    // For each vector component, copy value from "a" if "sel" is "false", or from "b" otherwise.
    NFE_FORCE_INLINE static const Vec8f Select(const Vec8f& a, const Vec8f& b, const BoolType& sel);

    template<uint32 selX, uint32 selY, uint32 selZ, uint32 selW>
    NFE_FORCE_INLINE static const Vec8f Select(const Vec8f& a, const Vec8f& b);

    // Check if the vector is equal to zero
    NFE_FORCE_INLINE bool IsZero() const;

    // Check if any component is NaN
    NFE_FORCE_INLINE bool IsNaN() const;

    // Check if any component is an infinity
    NFE_FORCE_INLINE bool IsInfinite() const;

    // Check if is not NaN or infinity
    NFE_FORCE_INLINE bool IsValid() const;

    // Check if two vectors are (almost) equal.
    NFE_FORCE_INLINE static bool AlmostEqual(const Vec8f& v1, const Vec8f& v2, float epsilon = NFE_MATH_EPSILON);

    // Fused multiply and add (a * b + c)
    NFE_FORCE_INLINE static const Vec8f MulAndAdd(const Vec8f& a, const Vec8f& b, const Vec8f& c);
    NFE_FORCE_INLINE static const Vec8f MulAndAdd(const Vec8f& a, const float b, const Vec8f& c);

    // Fused multiply and subtract (a * b - c)
    NFE_FORCE_INLINE static const Vec8f MulAndSub(const Vec8f& a, const Vec8f& b, const Vec8f& c);
    NFE_FORCE_INLINE static const Vec8f MulAndSub(const Vec8f& a, const float b, const Vec8f& c);

    // Fused multiply (negated) and add (a * b + c)
    // Fused multiply (negated) and add (a * b + c)
    NFE_FORCE_INLINE static const Vec8f NegMulAndAdd(const Vec8f& a, const Vec8f& b, const Vec8f& c);
    NFE_FORCE_INLINE static const Vec8f NegMulAndAdd(const Vec8f& a, const float b, const Vec8f& c);

    // Fused multiply (negated) and subtract (a * b - c)
    NFE_FORCE_INLINE static const Vec8f NegMulAndSub(const Vec8f& a, const Vec8f& b, const Vec8f& c);
    NFE_FORCE_INLINE static const Vec8f NegMulAndSub(const Vec8f& a, const float b, const Vec8f& c);

    // Calculate horizontal maximum. Result is splatted across all elements.
    NFE_FORCE_INLINE const Vec8f HorizontalMax() const;

    // Compute fmodf(x, 1.0f)
    NFE_FORCE_INLINE static const Vec8f Fmod1(const Vec8f& x);

    // transpose 8x8 matrix
    //NFE_FORCE_INLINE static void Transpose8x8(Vec8f& v0, Vec8f& v1, Vec8f& v2, Vec8f& v3, Vec8f& v4, Vec8f& v5, Vec8f& v6, Vec8f& v7);

private:

    friend struct Vec8i;

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
            Vec4f low;
            Vec4f high;
        };
#endif // NFE_USE_AVX
    };
};

// like Vec8f::operator * (float)
NFE_FORCE_INLINE const Vec8f operator*(float a, const Vec8f& b);

// some commonly used constants
NFE_GLOBAL_CONST Vec8f VECTOR8_EPSILON = { NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON };
NFE_GLOBAL_CONST Vec8f VECTOR8_HALVES = { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
NFE_GLOBAL_CONST Vec8f VECTOR8_MIN = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };
NFE_GLOBAL_CONST Vec8f VECTOR8_MAX = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
NFE_GLOBAL_CONST Vec8f VECTOR8_INF = { std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity() };
NFE_GLOBAL_CONST Vec8f VECTOR8_ONE = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
NFE_GLOBAL_CONST Vec8f VECTOR8_MINUS_ONE = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f };
NFE_GLOBAL_CONST Vec8f VECTOR8_MASK_X = { 0xFFFFFFFFu, 0u, 0u, 0u, 0u, 0u, 0u, 0u };
NFE_GLOBAL_CONST Vec8f VECTOR8_MASK_Y = { 0u, 0xFFFFFFFFu, 0u, 0u, 0u, 0u, 0u, 0u };
NFE_GLOBAL_CONST Vec8f VECTOR8_MASK_Z = { 0u, 0u, 0xFFFFFFFF, 0u, 0u, 0u, 0u, 0u };
NFE_GLOBAL_CONST Vec8f VECTOR8_MASK_W = { 0u, 0u, 0u, 0xFFFFFFFF, 0u, 0u, 0u, 0u };
NFE_GLOBAL_CONST Vec8f VECTOR8_MASK_ALL = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
NFE_GLOBAL_CONST Vec8f VECTOR8_MASK_ABS = { 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF };
NFE_GLOBAL_CONST Vec8f VECTOR8_MASK_SIGN = { 0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u };
NFE_GLOBAL_CONST Vec8f VECTOR8_INV_255 = { 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f };
NFE_GLOBAL_CONST Vec8f VECTOR8_255 = { 255.0f, 255.0f, 255.0f, 255.0f, 255.0f, 255.0f, 255.0f, 255.0f };

} // namespace Math
} // namespace NFE

#ifdef NFE_USE_AVX
#include "Vec8fImplAVX.hpp"
#else
#include "Vec8fImplNaive.hpp"
#endif // NFE_USE_AVX

#include "Vec8fImplCommon.hpp"
