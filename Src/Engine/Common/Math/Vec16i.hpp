#pragma once

#include "Math.hpp"
#include "Vec8i.hpp"

namespace NFE {
namespace Math {

struct Vec16f;

/**
 * Boolean vector for Vec16i and Vec16ui types.
 */
struct NFE_ALIGN(32) VecBool16i : public Common::Aligned<32>
{
    VecBool16i() = default;

    NFE_FORCE_INLINE explicit VecBool16i(bool scalar);
    NFE_FORCE_INLINE VecBool16i(const VecBool8i& low, const VecBool8i& high);

    NFE_FORCE_INLINE explicit VecBool16i(
        bool e0, bool e1, bool e2, bool e3, bool e4, bool e5, bool e6, bool e7,
        bool e8, bool e9, bool e10, bool e11, bool e12, bool e13, bool e14, bool e15);

#ifdef NFE_USE_AVX512
    NFE_FORCE_INLINE VecBool16i(const __mmask16 other) : mask(other) { }
    NFE_FORCE_INLINE explicit VecBool16i(const __m512 other) : mask(_mm512_movepi32_mask(_mm512_castps_si512(other))) { }
    NFE_FORCE_INLINE explicit VecBool16i(const __m512i other) : mask(_mm512_movepi32_mask(other)) { }
    NFE_FORCE_INLINE operator __mmask16() const { return mask; }
#endif // NFE_USE_AVX512

    template<uint32 index>
    NFE_FORCE_INLINE bool Get() const;

    NFE_FORCE_INLINE uint32 GetMask() const;

    NFE_FORCE_INLINE bool All() const;
    NFE_FORCE_INLINE bool None() const;
    NFE_FORCE_INLINE bool Any() const;

    NFE_FORCE_INLINE const VecBool16i operator & (const VecBool16i& rhs) const;
    NFE_FORCE_INLINE const VecBool16i operator | (const VecBool16i& rhs) const;
    NFE_FORCE_INLINE const VecBool16i operator ^ (const VecBool16i& rhs) const;

    NFE_FORCE_INLINE bool operator == (const VecBool16i& rhs) const;

private:
    friend struct Vec16i;
    friend struct Vec16ui;

#ifdef NFE_USE_AVX512
    __mmask16 mask;
#else
    VecBool8i low;
    VecBool8i high;
#endif // NFE_USE_AVX512
};

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
    NFE_FORCE_INLINE const VecBool16i operator == (const Vec16i& b) const;
    NFE_FORCE_INLINE const VecBool16i operator != (const Vec16i& b) const;

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
    NFE_FORCE_INLINE static const Vec16i Select(const Vec16i& a, const Vec16i& b, const VecBool16i& sel);

private:

#ifdef NFE_USE_AVX512
    __m512i v;
#else
    Vec8i low;
    Vec8i high;
#endif // NFE_USE_AVX512
};

static_assert(sizeof(Vec16i) == sizeof(int32) * 16, "Invalid sizeof Vec16i");


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
    NFE_FORCE_INLINE const VecBool16i operator == (const Vec16ui & b) const;
    NFE_FORCE_INLINE const VecBool16i operator != (const Vec16ui & b) const;

    NFE_FORCE_INLINE static const Vec16ui Min(const Vec16ui & a, const Vec16ui & b);
    NFE_FORCE_INLINE static const Vec16ui Max(const Vec16ui & a, const Vec16ui & b);
    NFE_FORCE_INLINE const Vec16ui Clamped(const Vec16ui & min, const Vec16ui & max) const;

    // cast from float vector (preserve bits)
    NFE_FORCE_INLINE static const Vec16ui Cast(const Vec16f & v);

    // convert to float vector
    NFE_FORCE_INLINE const Vec16f AsVec16f() const;

    // for each vector component, copy value from "a" if "sel" is false, or from "b" otherwise.
    NFE_FORCE_INLINE static const Vec16ui Select(const Vec16ui & a, const Vec16ui & b, const VecBool16i & sel);

private:

#ifdef NFE_USE_AVX512
    __m512i v;
#else
    Vec8ui low;
    Vec8ui high;
#endif // NFE_USE_AVX512
};

static_assert(sizeof(Vec16ui) == sizeof(uint32) * 16, "Invalid sizeof Vec16ui");

} // namespace Math
} // namespace NFE


#ifdef NFE_USE_AVX512
#include "Vec16iImplAVX512.hpp"
#else
#include "Vec16iImplNaive.hpp"
#endif // NFE_USE_AVX512
