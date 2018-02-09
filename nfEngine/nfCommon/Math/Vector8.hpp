#pragma once

#include "../nfCommon.hpp"
#include "Math.hpp"
#include "Vector4.hpp"

#include <immintrin.h>


namespace NFE {
namespace Math {

/**
 * 8-element SIMD vector.
 */
struct NFE_ALIGN(32) Vector8
{
    union
    {
        float f[8];
        int i[8];
        unsigned int u[8];
        __m256 v;
    };

    // conversion to/from AVX types
    NFE_INLINE operator __m256() const { return v; }
    NFE_INLINE operator __m256() { return v; }
    NFE_INLINE operator __m256i() const { return reinterpret_cast<const __m256i*>(&v)[0]; }

    // constructors
    NFE_INLINE Vector8();
    NFE_INLINE Vector8(const __m256& m);
    NFE_INLINE explicit Vector8(const Vector4& lo);
    NFE_INLINE Vector8(const Vector4& lo, const Vector4& hi);
    NFE_INLINE Vector8(float e0, float e1 = 0.0f, float e2 = 0.0f, float e3 = 0.0f, float e4 = 0.0f, float e5 = 0.0f, float e6 = 0.0f, float e7 = 0.0f);
    NFE_INLINE Vector8(int e0, int e1 = 0, int e2 = 0, int e3 = 0, int e4 = 0, int e5 = 0, int e6 = 0, int e7 = 0);
    NFE_INLINE Vector8(const float* src);
    NFE_INLINE void Set(float scalar);

    // element access
    NFE_INLINE float operator[] (int index) const
    {
        return f[index];
    }

    // element access (reference)
    NFE_INLINE float& operator[] (int index)
    {
        return f[index];
    }

    /// simple arithmetics
    NFE_INLINE Vector8 operator- () const;
    NFE_INLINE Vector8 operator+ (const Vector8& b) const;
    NFE_INLINE Vector8 operator- (const Vector8& b) const;
    NFE_INLINE Vector8 operator* (const Vector8& b) const;
    NFE_INLINE Vector8 operator/ (const Vector8& b) const;
    NFE_INLINE Vector8 operator* (float b) const;
    NFE_INLINE Vector8 operator/ (float b) const;
    NFE_INLINE Vector8& operator+= (const Vector8& b);
    NFE_INLINE Vector8& operator-= (const Vector8& b);
    NFE_INLINE Vector8& operator*= (const Vector8& b);
    NFE_INLINE Vector8& operator/= (const Vector8& b);
    NFE_INLINE Vector8& operator*= (float b);
    NFE_INLINE Vector8& operator/= (float b);

    /// comparison operators (returns true, if all the elements satisfy the equation)
    NFE_INLINE bool operator== (const Vector8& b) const;
    NFE_INLINE bool operator< (const Vector8& b) const;
    NFE_INLINE bool operator<= (const Vector8& b) const;
    NFE_INLINE bool operator> (const Vector8& b) const;
    NFE_INLINE bool operator>= (const Vector8& b) const;
    NFE_INLINE bool operator!= (const Vector8& b) const;

    /// bitwise logic operations
    NFE_INLINE Vector8 operator& (const Vector8& b) const;
    NFE_INLINE Vector8 operator| (const Vector8& b) const;
    NFE_INLINE Vector8 operator^ (const Vector8& b) const;
    NFE_INLINE Vector8& operator&= (const Vector8& b);
    NFE_INLINE Vector8& operator|= (const Vector8& b);
    NFE_INLINE Vector8& operator^= (const Vector8& b);
    NFE_INLINE static Vector8 Splat(float f);

    NFE_INLINE static Vector8 Floor(const Vector8& v);
    NFE_INLINE static Vector8 Sqrt(const Vector8& v);
    NFE_INLINE static Vector8 Reciprocal(const Vector8& v);
    NFE_INLINE static Vector8 FastReciprocal(const Vector8& v);
    NFE_INLINE static Vector8 Lerp(const Vector8& v1, const Vector8& v2, const Vector8& weight);
    NFE_INLINE static Vector8 Lerp(const Vector8& v1, const Vector8& v2, float weight);
    NFE_INLINE static Vector8 Min(const Vector8& a, const Vector8& b);
    NFE_INLINE static Vector8 Max(const Vector8& a, const Vector8& b);
    NFE_INLINE static Vector8 Abs(const Vector8& v);
    NFE_INLINE Vector8 Clamped(const Vector8& min, const Vector8& max) const;

    NFE_INLINE static int EqualMask(const Vector8& v1, const Vector8& v2);
    NFE_INLINE static int LessMask(const Vector8& v1, const Vector8& v2);
    NFE_INLINE static int LessEqMask(const Vector8& v1, const Vector8& v2);
    NFE_INLINE static int GreaterMask(const Vector8& v1, const Vector8& v2);
    NFE_INLINE static int GreaterEqMask(const Vector8& v1, const Vector8& v2);
    NFE_INLINE static int NotEqualMask(const Vector8& v1, const Vector8& v2);

    /**
     * Build mask of sign bits.
     */
    NFE_INLINE int GetSignMask() const;


    /**
     * For each vector component, copy value from "a" if "sel" > 0.0f, or from "b" otherwise.
     */
    NFE_INLINE static Vector8 SelectBySign(const Vector8& a, const Vector8& b, const Vector8& sel);

    /**
     * Check if two vectors are (almost) equal.
     */
    NFE_INLINE static bool AlmostEqual(const Vector8& v1, const Vector8& v2, float epsilon = NFE_MATH_EPSILON)
    {
        const Vector8 diff = Abs(v1 - v2);
        const Vector8 epsilonV = Vector8::Splat(epsilon);
        return diff < epsilonV;
    }

    /**
     * Fused multiply and add.
     * @return  a * b + c
     */
    NFE_INLINE static Vector8 MulAndAdd(const Vector8& a, const Vector8& b, const Vector8& c);

    /**
     * Fused multiply and subtract.
     * @return  a * b - c
     */
    NFE_INLINE static Vector8 MulAndSub(const Vector8& a, const Vector8& b, const Vector8& c);

    /**
     * Fused multiply (negated) and add.
     * @return  - a * b + c
     */
    NFE_INLINE static Vector8 NegMulAndAdd(const Vector8& a, const Vector8& b, const Vector8& c);

    /**
     * Fused multiply (negated) and subtract.
     * @return  - a * b - c
     */
    NFE_INLINE static Vector8 NegMulAndSub(const Vector8& a, const Vector8& b, const Vector8& c);

    /**
     * Calculate horizontal minimum. Result is splatted across all elements.
     */
    NFE_INLINE Vector8 HorizontalMin() const;

    /**
     * Calculate horizontal maximum. Result is splatted across all elements.
     */
    NFE_INLINE Vector8 HorizontalMax() const;
};

// like Vector8::operator * (float)
NFE_INLINE Vector8 operator*(float a, const Vector8& b);


} // namespace Math
} // namespace NFE


#include "Vector8Constants.hpp"
#include "Vector8Impl.hpp"