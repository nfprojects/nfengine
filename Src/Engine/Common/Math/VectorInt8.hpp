#pragma once

#include "Math.hpp"
#include "VectorInt4.hpp"

namespace NFE {
namespace Math {

struct Vector8;

// 8-element integer SIMD vector
struct NFE_ALIGN(32) VectorInt8
{
    // constructors
    VectorInt8() = default;
    NFE_FORCE_INLINE static const VectorInt8 Zero();
    NFE_FORCE_INLINE VectorInt8(const VectorInt8& other);
    NFE_FORCE_INLINE VectorInt8& operator = (const VectorInt8& other);
    NFE_FORCE_INLINE VectorInt8(const VectorInt4& lo, const VectorInt4& hi);
    NFE_FORCE_INLINE explicit VectorInt8(const int32 scalar);
    NFE_FORCE_INLINE explicit VectorInt8(const uint32 scalar);
    NFE_FORCE_INLINE VectorInt8(const int32 e0, const int32 e1, const int32 e2, const int32 e3, const int32 e4, const int32 e5, const int32 e6, const int32 e7);

#ifdef NFE_USE_AVX
    NFE_FORCE_INLINE VectorInt8(const __m256i & m);
    NFE_FORCE_INLINE explicit VectorInt8(const __m256 & m);
    NFE_FORCE_INLINE operator __m256i() const { return v; }
    NFE_FORCE_INLINE operator __m256() const { return _mm256_castsi256_ps(v); }
#endif // NFE_USE_AVX

    NFE_FORCE_INLINE int32 operator[] (const uint32 index) const { return i[index]; }
    NFE_FORCE_INLINE int32& operator[] (const uint32 index) { return i[index]; }

    // bitwise logic operations
    NFE_FORCE_INLINE const VectorInt8 operator & (const VectorInt8& b) const;
    NFE_FORCE_INLINE const VectorInt8 operator | (const VectorInt8& b) const;
    NFE_FORCE_INLINE const VectorInt8 operator ^ (const VectorInt8& b) const;
    NFE_FORCE_INLINE VectorInt8& operator &= (const VectorInt8& b);
    NFE_FORCE_INLINE VectorInt8& operator |= (const VectorInt8& b);
    NFE_FORCE_INLINE VectorInt8& operator ^= (const VectorInt8& b);

    // simple arithmetics
    NFE_FORCE_INLINE const VectorInt8 operator - () const;
    NFE_FORCE_INLINE const VectorInt8 operator + (const VectorInt8& b) const;
    NFE_FORCE_INLINE const VectorInt8 operator - (const VectorInt8& b) const;
    NFE_FORCE_INLINE const VectorInt8 operator * (const VectorInt8& b) const;
    NFE_FORCE_INLINE VectorInt8& operator += (const VectorInt8& b);
    NFE_FORCE_INLINE VectorInt8& operator -= (const VectorInt8& b);
    NFE_FORCE_INLINE VectorInt8& operator *= (const VectorInt8& b);
    NFE_FORCE_INLINE const VectorInt8 operator + (int32 b) const;
    NFE_FORCE_INLINE const VectorInt8 operator - (int32 b) const;
    NFE_FORCE_INLINE const VectorInt8 operator * (int32 b) const;
    NFE_FORCE_INLINE const VectorInt8 operator % (int32 b) const;
    NFE_FORCE_INLINE VectorInt8& operator += (int32 b);
    NFE_FORCE_INLINE VectorInt8& operator -= (int32 b);
    NFE_FORCE_INLINE VectorInt8& operator *= (int32 b);

    // bit shifting
    NFE_FORCE_INLINE const VectorInt8 operator << (const VectorInt8& b) const;
    NFE_FORCE_INLINE const VectorInt8 operator >> (const VectorInt8& b) const;
    NFE_FORCE_INLINE const VectorInt8 operator << (int32 b) const;
    NFE_FORCE_INLINE const VectorInt8 operator >> (int32 b) const;

    /// comparison operators (returns true, if all the elements satisfy the equation)
    NFE_FORCE_INLINE bool operator == (const VectorInt8& b) const;
    NFE_FORCE_INLINE bool operator != (const VectorInt8& b) const;

    NFE_FORCE_INLINE static const VectorInt8 Min(const VectorInt8& a, const VectorInt8& b);
    NFE_FORCE_INLINE static const VectorInt8 Max(const VectorInt8& a, const VectorInt8& b);
    NFE_FORCE_INLINE const VectorInt8 Clamped(const VectorInt8& min, const VectorInt8& max) const;

    // convert from float vector to integer vector
    NFE_FORCE_INLINE static const VectorInt8 Convert(const Vector8& v);

    // convert to float vector
    NFE_FORCE_INLINE const Vector8 ConvertToFloat() const;

    // cast from float vector (preserve bits)
    NFE_FORCE_INLINE static const VectorInt8 Cast(const Vector8& v);

    // convert to float vector
    NFE_FORCE_INLINE const Vector8 CastToFloat() const;

    // build mask of sign bits.
    NFE_FORCE_INLINE uint32 GetSignMask() const;

    // for each vector component, copy value from "a" if "sel" > 0.0f, or from "b" otherwise.
    NFE_FORCE_INLINE static const VectorInt8 SelectBySign(const VectorInt8& a, const VectorInt8& b, const VectorInt8& sel);

private:
    union
    {
        int32 i[8];
        uint32 u[8];

#ifdef NFE_USE_AVX
        __m256 f;
        __m256i v;
#endif // NFE_USE_AVX

        struct
        {
            VectorInt4 low;
            VectorInt4 high;
        };
    };
};

// gather 8 floats using base pointer and indices
NFE_FORCE_INLINE const Vector8 Gather8(const float* basePtr, const VectorInt8& indices);

} // namespace Math
} // namespace NFE


#ifdef NFE_USE_AVX2
#include "VectorInt8ImplAVX2.hpp"
#else
#include "VectorInt8ImplNaive.hpp"
#endif // NFE_USE_AVX2
