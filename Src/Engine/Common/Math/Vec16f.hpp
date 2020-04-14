#pragma once

#include "Math.hpp"
#include "Vec8f.hpp"

namespace NFE {
namespace Math {

/**
 * Boolean vector for Vec16f type.
 */
struct NFE_ALIGN(32) VecBool16 : public Common::Aligned<32>
{
    VecBool16() = default;

    NFE_FORCE_INLINE explicit VecBool16(bool scalar);

    NFE_FORCE_INLINE explicit VecBool16(
        bool e0, bool e1, bool e2, bool e3, bool e4, bool e5, bool e6, bool e7,
        bool e8, bool e9, bool e10, bool e11, bool e12, bool e13, bool e14, bool e15);

    NFE_FORCE_INLINE VecBool16(const __mmask16 other) : mask(other) { }
    NFE_FORCE_INLINE explicit VecBool16(const __m512 other) : mask(_mm512_movepi32_mask(_mm512_castps_si512(other))) { }
    NFE_FORCE_INLINE explicit VecBool16(const __m512i other) : mask(_mm512_movepi32_mask(other)) { }
    NFE_FORCE_INLINE operator __mmask16() const { return mask; }

    template<uint32 index>
    NFE_FORCE_INLINE bool Get() const;

    NFE_FORCE_INLINE uint16 GetMask() const;

    NFE_FORCE_INLINE bool All() const;
    NFE_FORCE_INLINE bool None() const;
    NFE_FORCE_INLINE bool Any() const;

    NFE_FORCE_INLINE const VecBool16 operator & (const VecBool16 rhs) const;
    NFE_FORCE_INLINE const VecBool16 operator | (const VecBool16 rhs) const;
    NFE_FORCE_INLINE const VecBool16 operator ^ (const VecBool16 rhs) const;

    NFE_FORCE_INLINE bool operator == (const VecBool16 rhs) const;

private:
    friend struct Vec16f;

    __mmask16 mask;
};

/**
 * 16-element SIMD float vector.
 */
struct NFE_ALIGN(64) Vec16f : public Common::Aligned<64>
{
    // constructors
    NFE_FORCE_INLINE Vec16f();
    NFE_FORCE_INLINE Vec16f(const Vec16f& other);
    NFE_FORCE_INLINE static const Vec16f Zero();
    NFE_FORCE_INLINE Vec16f(const Vec8f& lo, const Vec8f& hi);
    NFE_FORCE_INLINE explicit Vec16f(const float scalar);
    NFE_FORCE_INLINE explicit Vec16f(const int32 scalar);
    NFE_FORCE_INLINE explicit Vec16f(const uint32 scalar);
    NFE_FORCE_INLINE Vec16f(float e0, float e1, float e2, float e3, float e4, float e5, float e6, float e7, float e8, float e9, float e10, float e11, float e12, float e13, float e14, float e15);
    NFE_FORCE_INLINE Vec16f(const float* src);
    NFE_FORCE_INLINE Vec16f& operator = (const Vec16f& other);
    NFE_FORCE_INLINE static const Vec16f FromInteger(int32 x);

    // Rearrange vector elements (in both lanes, parallel)
    template<uint32 ix = 0, uint32 iy = 1, uint32 iz = 2, uint32 iw = 3>
    NFE_FORCE_INLINE const Vec16f Swizzle() const;

#ifdef NFE_USE_AVX512
    NFE_FORCE_INLINE Vec16f(const __m512 & m);
    NFE_FORCE_INLINE operator __m512() const { return v; }
    NFE_FORCE_INLINE operator __m512i() const { return reinterpret_cast<const __m512i*>(&v)[0]; }
#endif // NFE_USE_AVX

    NFE_FORCE_INLINE float operator[] (uint32 index) const
    {
        NFE_ASSERT(index < 16, "Index out of bounds (%u)", index);
        return f[index];
    }

    NFE_FORCE_INLINE float& operator[] (uint32 index)
    {
        NFE_ASSERT(index < 16, "Index out of bounds (%u)", index);
        return f[index];
    }

    // extract lower lanes
    NFE_FORCE_INLINE const Vec8f Low() const;
    // extract higher lanes
    NFE_FORCE_INLINE const Vec8f High() const;

    // simple arithmetics
    NFE_FORCE_INLINE const Vec16f operator - () const;
    NFE_FORCE_INLINE const Vec16f operator + (const Vec16f& b) const;
    NFE_FORCE_INLINE const Vec16f operator - (const Vec16f& b) const;
    NFE_FORCE_INLINE const Vec16f operator * (const Vec16f& b) const;
    NFE_FORCE_INLINE const Vec16f operator / (const Vec16f& b) const;
    NFE_FORCE_INLINE const Vec16f operator * (float b) const;
    NFE_FORCE_INLINE const Vec16f operator / (float b) const;
    NFE_FORCE_INLINE Vec16f& operator += (const Vec16f& b);
    NFE_FORCE_INLINE Vec16f& operator -= (const Vec16f& b);
    NFE_FORCE_INLINE Vec16f& operator *= (const Vec16f& b);
    NFE_FORCE_INLINE Vec16f& operator /= (const Vec16f& b);
    NFE_FORCE_INLINE Vec16f& operator *= (float b);
    NFE_FORCE_INLINE Vec16f& operator /= (float b);
    friend const Vec16f operator * (float a, const Vec16f& b);

    // comparison operators (returns true, if all the elements satisfy the equation)
    NFE_FORCE_INLINE const VecBool16 operator == (const Vec16f& b) const;
    NFE_FORCE_INLINE const VecBool16 operator < (const Vec16f& b) const;
    NFE_FORCE_INLINE const VecBool16 operator <= (const Vec16f& b) const;
    NFE_FORCE_INLINE const VecBool16 operator > (const Vec16f& b) const;
    NFE_FORCE_INLINE const VecBool16 operator >= (const Vec16f& b) const;
    NFE_FORCE_INLINE const VecBool16 operator != (const Vec16f& b) const;

    // bitwise logic operations
    NFE_FORCE_INLINE const Vec16f operator & (const Vec16f& b) const;
    NFE_FORCE_INLINE const Vec16f operator | (const Vec16f& b) const;
    NFE_FORCE_INLINE const Vec16f operator ^ (const Vec16f& b) const;
    NFE_FORCE_INLINE Vec16f& operator &= (const Vec16f& b);
    NFE_FORCE_INLINE Vec16f& operator |= (const Vec16f& b);
    NFE_FORCE_INLINE Vec16f& operator ^= (const Vec16f& b);

    NFE_FORCE_INLINE static const Vec16f Floor(const Vec16f& v);
    NFE_FORCE_INLINE static const Vec16f Sqrt(const Vec16f& v);
    NFE_FORCE_INLINE static const Vec16f Reciprocal(const Vec16f& v);
    NFE_FORCE_INLINE static const Vec16f FastReciprocal(const Vec16f& v);
    NFE_FORCE_INLINE static const Vec16f Lerp(const Vec16f& v1, const Vec16f& v2, const Vec16f& weight);
    NFE_FORCE_INLINE static const Vec16f Lerp(const Vec16f& v1, const Vec16f& v2, float weight);
    NFE_FORCE_INLINE static const Vec16f Min(const Vec16f& a, const Vec16f& b);
    NFE_FORCE_INLINE static const Vec16f Max(const Vec16f& a, const Vec16f& b);
    NFE_FORCE_INLINE static const Vec16f Abs(const Vec16f& v);
    NFE_FORCE_INLINE const Vec16f Clamped(const Vec16f& min, const Vec16f& max) const;

    // Build mask of sign bits.
    NFE_FORCE_INLINE uint32 GetSignMask() const;

    // For each vector component, copy value from "a" if "sel" is "false", or from "b" otherwise.
    NFE_FORCE_INLINE static const Vec16f Select(const Vec16f& a, const Vec16f& b, const VecBool16& sel);

    template<uint32 selX, uint32 selY, uint32 selZ, uint32 selW>
    NFE_FORCE_INLINE static const Vec16f Select(const Vec16f& a, const Vec16f& b);

    // Check if the vector is equal to zero
    NFE_FORCE_INLINE bool IsZero() const;

    // Check if any component is NaN
    NFE_FORCE_INLINE bool IsNaN() const;

    // Check if any component is an infinity
    NFE_FORCE_INLINE bool IsInfinite() const;

    // Check if is not NaN or infinity
    NFE_FORCE_INLINE bool IsValid() const;

    // Check if two vectors are (almost) equal.
    NFE_FORCE_INLINE static bool AlmostEqual(const Vec16f& v1, const Vec16f& v2, float epsilon = NFE_MATH_EPSILON);

    // Fused multiply and add (a * b + c)
    NFE_FORCE_INLINE static const Vec16f MulAndAdd(const Vec16f& a, const Vec16f& b, const Vec16f& c);
    NFE_FORCE_INLINE static const Vec16f MulAndAdd(const Vec16f& a, const float b, const Vec16f& c) { return MulAndAdd(a, Vec16f(b), c); }

    // Fused multiply and subtract (a * b - c)
    NFE_FORCE_INLINE static const Vec16f MulAndSub(const Vec16f& a, const Vec16f& b, const Vec16f& c);
    NFE_FORCE_INLINE static const Vec16f MulAndSub(const Vec16f& a, const float b, const Vec16f& c) { return MulAndSub(a, Vec16f(b), c); }

    // Fused multiply (negated) and add (a * b + c)
    NFE_FORCE_INLINE static const Vec16f NegMulAndAdd(const Vec16f& a, const Vec16f& b, const Vec16f& c);
    NFE_FORCE_INLINE static const Vec16f NegMulAndAdd(const Vec16f& a, const float b, const Vec16f& c) { return NegMulAndAdd(a, Vec16f(b), c); }

    // Fused multiply (negated) and subtract (a * b - c)
    NFE_FORCE_INLINE static const Vec16f NegMulAndSub(const Vec16f& a, const Vec16f& b, const Vec16f& c);
    NFE_FORCE_INLINE static const Vec16f NegMulAndSub(const Vec16f& a, const float b, const Vec16f& c) { return NegMulAndSub(a, Vec16f(b), c); }

    // transpose 8x8 matrix
    //NFE_FORCE_INLINE static void Transpose8x8(Vec16f& v0, Vec16f& v1, Vec16f& v2, Vec16f& v3, Vec16f& v4, Vec16f& v5, Vec16f& v6, Vec16f& v7);

private:

    friend struct Vec8i;

    union
    {
        float f[16];
        int32 i[16];
        uint32 u[16];
#ifdef NFE_USE_AVX512
        __m512 v;
#else
        struct
        {
            Vec16f low;
            Vec16f high;
        };
#endif // NFE_USE_AVX512
    };
};

// like Vec16f::operator * (float)
NFE_FORCE_INLINE const Vec16f operator*(float a, const Vec16f& b);

} // namespace Math
} // namespace NFE


#ifdef NFE_USE_AVX512
#include "Vec16fImplAVX512.hpp"
#else
#include "Vec16fImplNaive.hpp"
#endif // NFE_USE_AVX512
