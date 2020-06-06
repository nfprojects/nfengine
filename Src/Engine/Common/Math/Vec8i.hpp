#pragma once

#include "Math.hpp"
#include "Vec4i.hpp"

namespace NFE {
namespace Math {

struct Vec8f;

/**
 * Boolean vector for Vec4f type.
 */
struct NFE_ALIGN(32) VecBool8i : public Common::Aligned<16>
{
    VecBool8i() = default;
    VecBool8i(const VecBool8i& other) = default;
    NFE_FORCE_INLINE VecBool8i(const VecBool8f& other);
    NFE_FORCE_INLINE VecBool8i(bool e0, bool e1, bool e2, bool e3, bool e4, bool e5, bool e6, bool e7);

    template<uint32 index>
    NFE_FORCE_INLINE bool Get() const;

    NFE_FORCE_INLINE bool All() const;
    NFE_FORCE_INLINE bool None() const;
    NFE_FORCE_INLINE bool Any() const;

    NFE_FORCE_INLINE const VecBool8i operator & (const VecBool8i rhs) const;
    NFE_FORCE_INLINE const VecBool8i operator | (const VecBool8i rhs) const;
    NFE_FORCE_INLINE const VecBool8i operator ^ (const VecBool8i rhs) const;

    NFE_FORCE_INLINE bool operator == (const VecBool8i& other) const;
    NFE_FORCE_INLINE bool operator != (const VecBool8i& other) const;

private:
    friend struct Vec8i;
    friend struct Vec8ui;

#if defined(NFE_USE_AVX512)
    __mmask8 mask;
    NFE_FORCE_INLINE VecBool8i(const __mmask8 other) : mask(other) { }
    NFE_FORCE_INLINE explicit VecBool8i(const __m256 other) : mask(_mm256_movepi32_mask(_mm256_castps_si256(other))) { }
    NFE_FORCE_INLINE explicit VecBool8i(const __m256i other) : mask(_mm256_movepi32_mask(other)) { }
    NFE_FORCE_INLINE operator __mmask8() const { return mask; }
#elif defined(NFE_USE_AVX)
    NFE_FORCE_INLINE VecBool8i(const __m256 other) : v(_mm256_castps_si256(other)) { }
    NFE_FORCE_INLINE VecBool8i(const __m256i other) : v(other) { }
    NFE_FORCE_INLINE operator __m256() const { return _mm256_castsi256_ps(v); }
    NFE_FORCE_INLINE operator __m256i() const { return v; }
    __m256i v;
#else
    bool b[8];
#endif // NFE_USE_SSE
};

// 8-element integer SIMD vector
struct NFE_ALIGN(32) Vec8i : public Common::Aligned<32>
{
    friend struct Vec8ui;

    // constructors
    Vec8i() = default;
    NFE_FORCE_INLINE static const Vec8i Zero();
    NFE_FORCE_INLINE Vec8i(const Vec8i& other);
    NFE_FORCE_INLINE explicit Vec8i(const Vec8ui& other);
    NFE_FORCE_INLINE Vec8i& operator = (const Vec8i& other);
    NFE_FORCE_INLINE Vec8i(const Vec4i& lo, const Vec4i& hi);
    NFE_FORCE_INLINE explicit Vec8i(const int32 scalar);
    NFE_FORCE_INLINE explicit Vec8i(const int32* scalarPtr);
    NFE_FORCE_INLINE Vec8i(const int32 e0, const int32 e1, const int32 e2, const int32 e3, const int32 e4, const int32 e5, const int32 e6, const int32 e7);
    NFE_FORCE_INLINE static const Vec8i Iota(const int32 value);

#ifdef NFE_USE_AVX
    NFE_FORCE_INLINE Vec8i(const __m256i & m);
    NFE_FORCE_INLINE explicit Vec8i(const __m256 & m);
    NFE_FORCE_INLINE operator __m256i() const { return v; }
    NFE_FORCE_INLINE operator __m256() const { return _mm256_castsi256_ps(v); }
#endif // NFE_USE_AVX

    NFE_FORCE_INLINE int32 operator[] (const uint32 index) const
    {
        NFE_ASSERT(index < 8, "Index out of bounds (%u)", index);
        return i[index];
    }

    NFE_FORCE_INLINE int32& operator[] (const uint32 index)
    {
        NFE_ASSERT(index < 8, "Index out of bounds (%u)", index);
        return i[index];
    }

    // bitwise logic operations
    NFE_FORCE_INLINE const Vec8i operator & (const Vec8i& b) const;
    NFE_FORCE_INLINE const Vec8i operator | (const Vec8i& b) const;
    NFE_FORCE_INLINE const Vec8i operator ^ (const Vec8i& b) const;
    NFE_FORCE_INLINE Vec8i& operator &= (const Vec8i& b);
    NFE_FORCE_INLINE Vec8i& operator |= (const Vec8i& b);
    NFE_FORCE_INLINE Vec8i& operator ^= (const Vec8i& b);

    // simple arithmetics
    NFE_FORCE_INLINE const Vec8i operator - () const;
    NFE_FORCE_INLINE const Vec8i operator + (const Vec8i& b) const;
    NFE_FORCE_INLINE const Vec8i operator - (const Vec8i& b) const;
    NFE_FORCE_INLINE const Vec8i operator * (const Vec8i& b) const;
    NFE_FORCE_INLINE Vec8i& operator += (const Vec8i& b);
    NFE_FORCE_INLINE Vec8i& operator -= (const Vec8i& b);
    NFE_FORCE_INLINE Vec8i& operator *= (const Vec8i& b);
    NFE_FORCE_INLINE const Vec8i operator + (int32 b) const;
    NFE_FORCE_INLINE const Vec8i operator - (int32 b) const;
    NFE_FORCE_INLINE const Vec8i operator * (int32 b) const;
    NFE_FORCE_INLINE const Vec8i operator % (int32 b) const;
    NFE_FORCE_INLINE Vec8i& operator += (int32 b);
    NFE_FORCE_INLINE Vec8i& operator -= (int32 b);
    NFE_FORCE_INLINE Vec8i& operator *= (int32 b);

    // bit shifting
    NFE_FORCE_INLINE const Vec8i operator << (const Vec8i& b) const;
    NFE_FORCE_INLINE const Vec8i operator >> (const Vec8i& b) const;
    NFE_FORCE_INLINE const Vec8i operator << (int32 b) const;
    NFE_FORCE_INLINE const Vec8i operator >> (int32 b) const;

    /// comparison operators
    NFE_FORCE_INLINE const VecBool8i operator == (const Vec8i& b) const;
    NFE_FORCE_INLINE const VecBool8i operator != (const Vec8i& b) const;

    NFE_FORCE_INLINE static const Vec8i Min(const Vec8i& a, const Vec8i& b);
    NFE_FORCE_INLINE static const Vec8i Max(const Vec8i& a, const Vec8i& b);
    NFE_FORCE_INLINE const Vec8i Clamped(const Vec8i& min, const Vec8i& max) const;

    // convert from float vector to integer vector
    NFE_FORCE_INLINE static const Vec8i Convert(const Vec8f& v);

    // convert to float vector
    NFE_FORCE_INLINE const Vec8f ConvertToVec8f() const;

    // cast from float vector (preserve bits)
    NFE_FORCE_INLINE static const Vec8i Cast(const Vec8f& v);

    // convert to float vector
    NFE_FORCE_INLINE const Vec8f AsVec8f() const;

    // build mask of sign bits.
    NFE_FORCE_INLINE uint32 GetSignMask() const;

    // for each vector component, copy value from "a" if "sel" > 0.0f, or from "b" otherwise.
    NFE_FORCE_INLINE static const Vec8i SelectBySign(const Vec8i& a, const Vec8i& b, const VecBool8i& sel);

private:

    NFE_UNNAMED_STRUCT union
    {
        int32 i[8];
        uint32 u[8];

#ifdef NFE_USE_AVX
        __m256 f;
        __m256i v;
#endif // NFE_USE_AVX

        NFE_UNNAMED_STRUCT struct
        {
            Vec4i low;
            Vec4i high;
        };
    };
};

// 8-element integer SIMD vector
struct NFE_ALIGN(32) Vec8ui : public Common::Aligned<32>
{
    friend struct Vec8i;

    NFE_UNNAMED_STRUCT union
    {
        int32 i[8];
        uint32 u[8];

#ifdef NFE_USE_AVX
        __m256 f;
        __m256i v;
#endif // NFE_USE_AVX

        NFE_UNNAMED_STRUCT struct
        {
            Vec4i low;
            Vec4i high;
        };
    };

    // constructors
    Vec8ui() = default;
    NFE_FORCE_INLINE static const Vec8ui Zero();
    NFE_FORCE_INLINE Vec8ui(const Vec8ui & other);
    NFE_FORCE_INLINE explicit Vec8ui(const Vec8i & other);
    NFE_FORCE_INLINE Vec8ui& operator = (const Vec8ui & other);
    NFE_FORCE_INLINE Vec8ui(const Vec4ui & lo, const Vec4ui & hi);
    NFE_FORCE_INLINE explicit Vec8ui(const uint32 scalar);
    NFE_FORCE_INLINE explicit Vec8ui(const uint32* scalarPtr);
    NFE_FORCE_INLINE Vec8ui(const uint32 e0, const uint32 e1, const uint32 e2, const uint32 e3, const uint32 e4, const uint32 e5, const uint32 e6, const uint32 e7);
    NFE_FORCE_INLINE static const Vec8ui Iota(const uint32 value);

#ifdef NFE_USE_AVX
    NFE_FORCE_INLINE Vec8ui(const __m256i & m);
    NFE_FORCE_INLINE explicit Vec8ui(const __m256 & m);
    NFE_FORCE_INLINE operator __m256i() const { return v; }
    NFE_FORCE_INLINE operator __m256() const { return _mm256_castsi256_ps(v); }
#endif // NFE_USE_AVX

    NFE_FORCE_INLINE int32 operator[] (const uint32 index) const
    {
        NFE_ASSERT(index < 8, "Index out of bounds (%u)", index);
        return i[index];
    }

    NFE_FORCE_INLINE int32& operator[] (const uint32 index)
    {
        NFE_ASSERT(index < 8, "Index out of bounds (%u)", index);
        return i[index];
    }

    // bitwise logic operations
    NFE_FORCE_INLINE const Vec8ui operator & (const Vec8ui & b) const;
    NFE_FORCE_INLINE const Vec8ui operator | (const Vec8ui & b) const;
    NFE_FORCE_INLINE const Vec8ui operator ^ (const Vec8ui & b) const;
    NFE_FORCE_INLINE Vec8ui& operator &= (const Vec8ui & b);
    NFE_FORCE_INLINE Vec8ui& operator |= (const Vec8ui & b);
    NFE_FORCE_INLINE Vec8ui& operator ^= (const Vec8ui & b);

    // simple arithmetics
    NFE_FORCE_INLINE const Vec8ui operator + (const Vec8ui & b) const;
    NFE_FORCE_INLINE const Vec8ui operator - (const Vec8ui & b) const;
    NFE_FORCE_INLINE Vec8ui& operator += (const Vec8ui & b);
    NFE_FORCE_INLINE Vec8ui& operator -= (const Vec8ui & b);
    NFE_FORCE_INLINE const Vec8ui operator + (int32 b) const;
    NFE_FORCE_INLINE const Vec8ui operator - (int32 b) const;
    NFE_FORCE_INLINE Vec8ui& operator += (int32 b);
    NFE_FORCE_INLINE Vec8ui& operator -= (int32 b);

    // bit shifting
    NFE_FORCE_INLINE const Vec8ui operator << (const Vec8ui & b) const;
    NFE_FORCE_INLINE const Vec8ui operator >> (const Vec8ui & b) const;
    NFE_FORCE_INLINE const Vec8ui operator << (int32 b) const;
    NFE_FORCE_INLINE const Vec8ui operator >> (int32 b) const;

    /// comparison operators
    NFE_FORCE_INLINE const VecBool8i operator == (const Vec8ui & b) const;
    NFE_FORCE_INLINE const VecBool8i operator != (const Vec8ui & b) const;

    NFE_FORCE_INLINE static const Vec8ui Min(const Vec8ui & a, const Vec8ui & b);
    NFE_FORCE_INLINE static const Vec8ui Max(const Vec8ui & a, const Vec8ui & b);
    NFE_FORCE_INLINE const Vec8ui Clamped(const Vec8ui & min, const Vec8ui & max) const;

    // cast from float vector (preserve bits)
    NFE_FORCE_INLINE static const Vec8ui Cast(const Vec8f & v);

    // convert to float vector
    NFE_FORCE_INLINE const Vec8f AsVec8f() const;

    // for each vector component, copy value from "a" if "sel" > 0.0f, or from "b" otherwise.
    NFE_FORCE_INLINE static const Vec8ui SelectBySign(const Vec8ui & a, const Vec8ui & b, const VecBool8i & sel);
};

// gather 8 floats using base pointer and indices
NFE_FORCE_INLINE const Vec8f Gather8(const float* basePtr, const Vec8i& indices);

} // namespace Math
} // namespace NFE


#ifdef NFE_USE_AVX2
#include "Vec8iImplAVX2.hpp"
#else
#include "Vec8iImplNaive.hpp"
#endif // NFE_USE_AVX2
