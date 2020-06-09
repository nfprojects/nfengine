#pragma once

#include "Math.hpp"
#include "Vec8i.hpp"

namespace NFE {
namespace Math {

struct Vec16f;
struct VecBool16;

// 16-element integer SIMD vector
struct NFE_ALIGN(64) Vec16i : public Common::Aligned<64>
{
    friend struct Vec16ui;

    // constructors
    Vec16i() = default;
    NFE_FORCE_INLINE static const Vec16i Zero();
    NFE_FORCE_INLINE Vec16i(const Vec16i& other);
    NFE_FORCE_INLINE Vec16i(const Vec8i& low, const Vec8i& high);
    NFE_FORCE_INLINE explicit Vec16i(const Vec16ui& other);
    NFE_FORCE_INLINE Vec16i& operator = (const Vec16i& other);
    NFE_FORCE_INLINE explicit Vec16i(const int32 scalar);
    NFE_FORCE_INLINE explicit Vec16i(const int32* scalarPtr);
    NFE_FORCE_INLINE Vec16i(const int32 e0, const int32 e1, const int32 e2, const int32 e3,
        const int32 e4, const int32 e5, const int32 e6, const int32 e7,
        const int32 e8, const int32 e9, const int32 e10, const int32 e11,
        const int32 e12, const int32 e13, const int32 e14, const int32 e15);
    NFE_FORCE_INLINE static const Vec16i Iota(const int32 value);

#ifdef NFE_USE_AVX512
    NFE_FORCE_INLINE Vec16i(const __m512i& m);
    NFE_FORCE_INLINE explicit Vec16i(const __m512& m);
    NFE_FORCE_INLINE operator __m512i() const { return v; }
#endif // NFE_USE_AVX512

    NFE_FORCE_INLINE int32 operator[] (const uint32 index) const
    {
        NFE_ASSERT(index < 16, "Index out of bounds (%u)", index);
        return reinterpret_cast<const int32*>(this)[index];
    }

    NFE_FORCE_INLINE int32& operator[] (const uint32 index)
    {
        NFE_ASSERT(index < 16, "Index out of bounds (%u)", index);
        return reinterpret_cast<int32*>(this)[index];
    }

    // bitwise logic operations
    NFE_FORCE_INLINE const Vec16i operator & (const Vec16i& b) const;
    NFE_FORCE_INLINE const Vec16i operator | (const Vec16i& b) const;
    NFE_FORCE_INLINE const Vec16i operator ^ (const Vec16i& b) const;
    NFE_FORCE_INLINE Vec16i& operator &= (const Vec16i& b);
    NFE_FORCE_INLINE Vec16i& operator |= (const Vec16i& b);
    NFE_FORCE_INLINE Vec16i& operator ^= (const Vec16i& b);

    // simple arithmetics
    NFE_FORCE_INLINE const Vec16i operator - () const;
    NFE_FORCE_INLINE const Vec16i operator + (const Vec16i& b) const;
    NFE_FORCE_INLINE const Vec16i operator - (const Vec16i& b) const;
    NFE_FORCE_INLINE const Vec16i operator * (const Vec16i& b) const;
    NFE_FORCE_INLINE Vec16i& operator += (const Vec16i& b);
    NFE_FORCE_INLINE Vec16i& operator -= (const Vec16i& b);
    NFE_FORCE_INLINE Vec16i& operator *= (const Vec16i& b);
    NFE_FORCE_INLINE const Vec16i operator + (int32 b) const;
    NFE_FORCE_INLINE const Vec16i operator - (int32 b) const;
    NFE_FORCE_INLINE const Vec16i operator * (int32 b) const;
    NFE_FORCE_INLINE Vec16i& operator += (int32 b);
    NFE_FORCE_INLINE Vec16i& operator -= (int32 b);
    NFE_FORCE_INLINE Vec16i& operator *= (int32 b);

    // bit shifting
    NFE_FORCE_INLINE const Vec16i operator << (const Vec16i& b) const;
    NFE_FORCE_INLINE const Vec16i operator >> (const Vec16i& b) const;
    NFE_FORCE_INLINE const Vec16i operator << (int32 b) const;
    NFE_FORCE_INLINE const Vec16i operator >> (int32 b) const;

    /// comparison operators
    NFE_FORCE_INLINE const VecBool16 operator == (const Vec16i& b) const;
    NFE_FORCE_INLINE const VecBool16 operator != (const Vec16i& b) const;

    NFE_FORCE_INLINE static const Vec16i Min(const Vec16i& a, const Vec16i& b);
    NFE_FORCE_INLINE static const Vec16i Max(const Vec16i& a, const Vec16i& b);
    NFE_FORCE_INLINE const Vec16i Clamped(const Vec16i& min, const Vec16i& max) const;

    // convert from float vector to integer vector
    NFE_FORCE_INLINE static const Vec16i Convert(const Vec16f& v);

    // convert to float vector
    NFE_FORCE_INLINE const Vec16f ConvertToVec16f() const;

    // cast from float vector (preserve bits)
    NFE_FORCE_INLINE static const Vec16i Cast(const Vec16f& v);

    // convert to float vector
    NFE_FORCE_INLINE const Vec16f AsVec16f() const;

    // build mask of sign bits.
    NFE_FORCE_INLINE uint32 GetSignMask() const;

    // for each vector component, copy value from "a" if "sel" is false, or from "b" otherwise.
    NFE_FORCE_INLINE static const Vec16i Select(const Vec16i& a, const Vec16i& b, const VecBool16& sel);

private:

#ifdef NFE_USE_AVX512
    __m512i v;
#else
    Vec8i low;
    Vec8i high;
#endif // NFE_USE_AVX512
};

// 8-element integer SIMD vector
struct NFE_ALIGN(64) Vec16ui : public Common::Aligned<64>
{
    friend struct Vec16i;

    // constructors
    Vec16ui() = default;
    NFE_FORCE_INLINE static const Vec16ui Zero();
    NFE_FORCE_INLINE Vec16ui(const Vec16ui& other);
    NFE_FORCE_INLINE explicit Vec16ui(const Vec16i& other);
    NFE_FORCE_INLINE Vec16ui(const Vec8ui& low, const Vec8ui& high);
    NFE_FORCE_INLINE Vec16ui& operator = (const Vec16ui& other);
    NFE_FORCE_INLINE explicit Vec16ui(const uint32 scalar);
    NFE_FORCE_INLINE explicit Vec16ui(const uint32* scalarPtr);
    NFE_FORCE_INLINE Vec16ui(const uint32 e0, const uint32 e1, const uint32 e2, const uint32 e3,
        const uint32 e4, const uint32 e5, const uint32 e6, const uint32 e7,
        const uint32 e8, const uint32 e9, const uint32 e10, const uint32 e11,
        const uint32 e12, const uint32 e13, const uint32 e14, const uint32 e15);
    NFE_FORCE_INLINE static const Vec16ui Iota(const uint32 value);

#ifdef NFE_USE_AVX512
    NFE_FORCE_INLINE Vec16ui(const __m512i & m);
    NFE_FORCE_INLINE explicit Vec16ui(const __m256 & m);
    NFE_FORCE_INLINE operator __m512i() const { return v; }
#endif // NFE_USE_AVX512

    NFE_FORCE_INLINE uint32 operator[] (const uint32 index) const
    {
        NFE_ASSERT(index < 16, "Index out of bounds (%u)", index);
        return reinterpret_cast<const uint32*>(this)[index];
    }

    NFE_FORCE_INLINE uint32& operator[] (const uint32 index)
    {
        NFE_ASSERT(index < 16, "Index out of bounds (%u)", index);
        return reinterpret_cast<uint32*>(this)[index];
    }

    // bitwise logic operations
    NFE_FORCE_INLINE const Vec16ui operator & (const Vec16ui & b) const;
    NFE_FORCE_INLINE const Vec16ui operator | (const Vec16ui & b) const;
    NFE_FORCE_INLINE const Vec16ui operator ^ (const Vec16ui & b) const;
    NFE_FORCE_INLINE Vec16ui& operator &= (const Vec16ui & b);
    NFE_FORCE_INLINE Vec16ui& operator |= (const Vec16ui & b);
    NFE_FORCE_INLINE Vec16ui& operator ^= (const Vec16ui & b);

    // simple arithmetics
    NFE_FORCE_INLINE const Vec16ui operator + (const Vec16ui & b) const;
    NFE_FORCE_INLINE const Vec16ui operator - (const Vec16ui & b) const;
    NFE_FORCE_INLINE Vec16ui& operator += (const Vec16ui & b);
    NFE_FORCE_INLINE Vec16ui& operator -= (const Vec16ui & b);
    NFE_FORCE_INLINE const Vec16ui operator + (uint32 b) const;
    NFE_FORCE_INLINE const Vec16ui operator - (uint32 b) const;
    NFE_FORCE_INLINE Vec16ui& operator += (uint32 b);
    NFE_FORCE_INLINE Vec16ui& operator -= (uint32 b);

    // bit shifting
    NFE_FORCE_INLINE const Vec16ui operator << (const Vec16ui & b) const;
    NFE_FORCE_INLINE const Vec16ui operator >> (const Vec16ui & b) const;
    NFE_FORCE_INLINE const Vec16ui operator << (uint32 b) const;
    NFE_FORCE_INLINE const Vec16ui operator >> (uint32 b) const;

    /// comparison operators
    NFE_FORCE_INLINE const VecBool16 operator == (const Vec16ui & b) const;
    NFE_FORCE_INLINE const VecBool16 operator != (const Vec16ui & b) const;

    NFE_FORCE_INLINE static const Vec16ui Min(const Vec16ui & a, const Vec16ui & b);
    NFE_FORCE_INLINE static const Vec16ui Max(const Vec16ui & a, const Vec16ui & b);
    NFE_FORCE_INLINE const Vec16ui Clamped(const Vec16ui & min, const Vec16ui & max) const;

    // cast from float vector (preserve bits)
    NFE_FORCE_INLINE static const Vec16ui Cast(const Vec16f & v);

    // convert to float vector
    NFE_FORCE_INLINE const Vec16f AsVec16f() const;

    // for each vector component, copy value from "a" if "sel" is false, or from "b" otherwise.
    NFE_FORCE_INLINE static const Vec16ui Select(const Vec16ui & a, const Vec16ui & b, const VecBool16 & sel);

private:

#ifdef NFE_USE_AVX512
    __m512i v;
#else
    Vec8ui low;
    Vec8ui high;
#endif // NFE_USE_AVX512
};


} // namespace Math
} // namespace NFE


#ifdef NFE_USE_AVX512
#include "Vec16iImplAVX512.hpp"
#else
#include "Vec16iImplNaive.hpp"
#endif // NFE_USE_AVX512
