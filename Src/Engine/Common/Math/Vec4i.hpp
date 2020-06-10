#pragma once

#include "Math.hpp"
#include "../Memory/Aligned.hpp"

namespace NFE {
namespace Math {

struct Vec4f;

/**
 * Boolean vector for Vec4f type.
 */
struct NFE_ALIGN(16) VecBool4i : public Common::Aligned<16>
{
    VecBool4i() = default;

    NFE_FORCE_INLINE explicit VecBool4i(bool scalar);
    NFE_FORCE_INLINE VecBool4i(bool x, bool y, bool z, bool w);
    NFE_FORCE_INLINE VecBool4i(int32 x, int32 y, int32 z, int32 w);

    NFE_FORCE_INLINE VecBool4i(const VecBool4f& other);
    NFE_FORCE_INLINE operator VecBool4f();

#ifdef NFE_USE_SSE
    NFE_FORCE_INLINE VecBool4i(const __m128 other) : v(_mm_castps_si128(other)) { }
    NFE_FORCE_INLINE VecBool4i(const __m128i other) : v(other) { }
    NFE_FORCE_INLINE operator __m128() const { return _mm_castsi128_ps(v); }
    NFE_FORCE_INLINE operator __m128i() const { return v; }
#endif // NFE_USE_SSE

    template<uint32 index>
    NFE_FORCE_INLINE bool Get() const;

    NFE_FORCE_INLINE uint32 GetMask() const;

    NFE_FORCE_INLINE bool All() const;
    NFE_FORCE_INLINE bool None() const;
    NFE_FORCE_INLINE bool Any() const;

    NFE_FORCE_INLINE bool All3() const;
    NFE_FORCE_INLINE bool None3() const;
    NFE_FORCE_INLINE bool Any3() const;

    NFE_FORCE_INLINE const VecBool4i operator & (const VecBool4i rhs) const;
    NFE_FORCE_INLINE const VecBool4i operator | (const VecBool4i rhs) const;
    NFE_FORCE_INLINE const VecBool4i operator ^ (const VecBool4i rhs) const;

    NFE_FORCE_INLINE bool operator == (const VecBool4i& other) const;
    NFE_FORCE_INLINE bool operator != (const VecBool4i& other) const;

    template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
    NFE_FORCE_INLINE const VecBool4i Swizzle() const;

private:
    friend struct Vec4i;
    friend struct Vec4ui;
    friend struct VecBool4f;

#ifdef NFE_USE_SSE
    __m128i v;
#else
    bool b[4];
#endif // NFE_USE_SSE
};

///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * 4-element signed integer SIMD vector.
 */
struct NFE_ALIGN(16) Vec4i : public Common::Aligned<16>
{
    NFE_UNNAMED_STRUCT union
    {
        int32 i[4];
        int64 i64[2];


#ifdef NFE_USE_SSE
        __m128i v;
#endif // NFE_USE_SSE

        NFE_UNNAMED_STRUCT struct
        {
            int32 x;
            int32 y;
            int32 z;
            int32 w;
        };
    };

    // constructors
    NFE_FORCE_INLINE Vec4i() = default;
    NFE_FORCE_INLINE static const Vec4i Zero();
    NFE_FORCE_INLINE Vec4i(const Vec4i& other);
    NFE_FORCE_INLINE explicit Vec4i(const Vec4ui& other);
    NFE_FORCE_INLINE Vec4i(const VecBool4i& other);
    NFE_FORCE_INLINE explicit Vec4i(const int32 scalar);
    NFE_FORCE_INLINE explicit Vec4i(const int32* scalarPtr);
    NFE_FORCE_INLINE Vec4i(const int32 x, const int32 y, const int32 z, const int32 w);
    NFE_FORCE_INLINE static const Vec4i Iota(const int32 value);
    
    NFE_FORCE_INLINE Vec4i& operator = (const Vec4i& other);

#ifdef NFE_USE_SSE
    NFE_FORCE_INLINE Vec4i(const __m128i& m);
    NFE_FORCE_INLINE operator __m128i() const { return v; }
#endif // NFE_USE_SSE

    NFE_FORCE_INLINE int32 operator[] (const uint32 index) const
    {
        NFE_ASSERT(index < 4, "Index out of bounds (%u)", index);
        return i[index];
    }

    NFE_FORCE_INLINE int32& operator[] (const uint32 index)
    {
        NFE_ASSERT(index < 4, "Index out of bounds (%u)", index);
        return i[index];
    }

    // bitwise logic operations
    NFE_FORCE_INLINE const Vec4i operator & (const Vec4i& b) const;
    NFE_FORCE_INLINE const Vec4i operator | (const Vec4i& b) const;
    NFE_FORCE_INLINE const Vec4i operator ^ (const Vec4i& b) const;
    NFE_FORCE_INLINE Vec4i& operator &= (const Vec4i& b);
    NFE_FORCE_INLINE Vec4i& operator |= (const Vec4i& b);
    NFE_FORCE_INLINE Vec4i& operator ^= (const Vec4i& b);
    NFE_FORCE_INLINE static const Vec4i AndNot(const Vec4i& a, const Vec4i& b);

    // simple arithmetics
    NFE_FORCE_INLINE const Vec4i operator - () const;
    NFE_FORCE_INLINE const Vec4i operator + (const Vec4i& b) const;
    NFE_FORCE_INLINE const Vec4i operator - (const Vec4i& b) const;
    NFE_FORCE_INLINE const Vec4i operator * (const Vec4i& b) const;
    NFE_FORCE_INLINE Vec4i& operator += (const Vec4i& b);
    NFE_FORCE_INLINE Vec4i& operator -= (const Vec4i& b);
    NFE_FORCE_INLINE Vec4i& operator *= (const Vec4i& b);
    NFE_FORCE_INLINE const Vec4i operator + (int32 b) const;
    NFE_FORCE_INLINE const Vec4i operator - (int32 b) const;
    NFE_FORCE_INLINE const Vec4i operator * (int32 b) const;
    NFE_FORCE_INLINE Vec4i& operator += (int32 b);
    NFE_FORCE_INLINE Vec4i& operator -= (int32 b);
    NFE_FORCE_INLINE Vec4i& operator *= (int32 b);

    // bit shifting
    NFE_FORCE_INLINE const Vec4i operator << (const Vec4i& b) const;
    NFE_FORCE_INLINE const Vec4i operator >> (const Vec4i& b) const;
    NFE_FORCE_INLINE Vec4i& operator <<= (const Vec4i& b);
    NFE_FORCE_INLINE Vec4i& operator >>= (const Vec4i& b);
    NFE_FORCE_INLINE const Vec4i operator << (int32 b) const;
    NFE_FORCE_INLINE const Vec4i operator >> (int32 b) const;
    NFE_FORCE_INLINE Vec4i& operator <<= (int32 b);
    NFE_FORCE_INLINE Vec4i& operator >>= (int32 b);

    // For each vector component, copy value from "a" if "sel" is "false", or from "b" otherwise
    NFE_FORCE_INLINE static const Vec4i Select(const Vec4i& a, const Vec4i& b, const VecBool4i& sel);

    template<uint32 selX, uint32 selY, uint32 selZ, uint32 selW>
    NFE_FORCE_INLINE static const Vec4i Select(const Vec4i& a, const Vec4i& b);

    NFE_FORCE_INLINE const VecBool4i operator == (const Vec4i& b) const;
    NFE_FORCE_INLINE const VecBool4i operator < (const Vec4i& b) const;
    NFE_FORCE_INLINE const VecBool4i operator <= (const Vec4i& b) const;
    NFE_FORCE_INLINE const VecBool4i operator > (const Vec4i& b) const;
    NFE_FORCE_INLINE const VecBool4i operator >= (const Vec4i& b) const;
    NFE_FORCE_INLINE const VecBool4i operator != (const Vec4i& b) const;

    NFE_FORCE_INLINE static const Vec4i Min(const Vec4i& a, const Vec4i& b);
    NFE_FORCE_INLINE static const Vec4i Max(const Vec4i& a, const Vec4i& b);

    NFE_FORCE_INLINE const Vec4i Clamped(const Vec4i& min, const Vec4i& max) const;

    // convert from float vector to integer vector (with rounding)
    NFE_FORCE_INLINE static const Vec4i Convert(const Vec4f& v);

    // convert from float vector to integer vector (with truncation towards zero)
    NFE_FORCE_INLINE static const Vec4i TruncateAndConvert(const Vec4f& v);

    // convert to float vector
    NFE_FORCE_INLINE const Vec4f ConvertToVec4f() const;

    // cast from float vector (preserve bits)
    NFE_FORCE_INLINE static const Vec4i Cast(const Vec4f& v);

    // convert to float vector
    NFE_FORCE_INLINE const Vec4f AsVec4f() const;

    // Rearrange vector elements
    template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
    NFE_FORCE_INLINE const Vec4i Swizzle() const;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * 4-element unsigned integer SIMD vector.
 */
struct NFE_ALIGN(16) Vec4ui : public Common::Aligned<16>
{
    NFE_UNNAMED_STRUCT union
    {
        uint32 u[4];
        uint64 u64[2];


#ifdef NFE_USE_SSE
        __m128i v;
#endif // NFE_USE_SSE

        NFE_UNNAMED_STRUCT struct
        {
            uint32 x;
            uint32 y;
            uint32 z;
            uint32 w;
        };
    };

    // constructors
    NFE_FORCE_INLINE Vec4ui() = default;
    NFE_FORCE_INLINE static const Vec4ui Zero();
    NFE_FORCE_INLINE Vec4ui(const Vec4ui& other);
    NFE_FORCE_INLINE Vec4ui(const VecBool4i& other);
    NFE_FORCE_INLINE explicit Vec4ui(const Vec4i& other);
    NFE_FORCE_INLINE explicit Vec4ui(const uint32 scalar);
    NFE_FORCE_INLINE explicit Vec4ui(const uint32* scalarPtr);
    NFE_FORCE_INLINE Vec4ui(const uint32 x, const uint32 y, const uint32 z, const uint32 w);
    NFE_FORCE_INLINE static const Vec4ui Iota(const uint32 value);

    NFE_FORCE_INLINE Vec4ui& operator = (const Vec4ui& other);

#ifdef NFE_USE_SSE
    NFE_FORCE_INLINE Vec4ui(const __m128i& m);
    NFE_FORCE_INLINE operator __m128i() const { return v; }
#endif // NFE_USE_SSE

    NFE_FORCE_INLINE uint32 operator[] (const uint32 index) const
    {
        NFE_ASSERT(index < 4, "Index out of bounds (%u)", index);
        return u[index];
    }

    NFE_FORCE_INLINE uint32& operator[] (const uint32 index)
    {
        NFE_ASSERT(index < 4, "Index out of bounds (%u)", index);
        return u[index];
    }

    // bitwise logic operations
    NFE_FORCE_INLINE const Vec4ui operator & (const Vec4ui& b) const;
    NFE_FORCE_INLINE const Vec4ui operator | (const Vec4ui& b) const;
    NFE_FORCE_INLINE const Vec4ui operator ^ (const Vec4ui& b) const;
    NFE_FORCE_INLINE Vec4ui& operator &= (const Vec4ui& b);
    NFE_FORCE_INLINE Vec4ui& operator |= (const Vec4ui& b);
    NFE_FORCE_INLINE Vec4ui& operator ^= (const Vec4ui& b);
    NFE_FORCE_INLINE static const Vec4ui AndNot(const Vec4ui & a, const Vec4ui & b);

    // simple arithmetics
    NFE_FORCE_INLINE const Vec4ui operator - () const;
    NFE_FORCE_INLINE const Vec4ui operator + (const Vec4ui& b) const;
    NFE_FORCE_INLINE const Vec4ui operator - (const Vec4ui& b) const;
    NFE_FORCE_INLINE const Vec4ui operator * (const Vec4ui& b) const;
    NFE_FORCE_INLINE Vec4ui& operator += (const Vec4ui& b);
    NFE_FORCE_INLINE Vec4ui& operator -= (const Vec4ui& b);
    NFE_FORCE_INLINE Vec4ui& operator *= (const Vec4ui& b);
    NFE_FORCE_INLINE const Vec4ui operator + (uint32 b) const;
    NFE_FORCE_INLINE const Vec4ui operator - (uint32 b) const;
    NFE_FORCE_INLINE const Vec4ui operator * (uint32 b) const;
    NFE_FORCE_INLINE Vec4ui& operator += (uint32 b);
    NFE_FORCE_INLINE Vec4ui& operator -= (uint32 b);
    NFE_FORCE_INLINE Vec4ui& operator *= (uint32 b);

    // bit shifting
    NFE_FORCE_INLINE const Vec4ui operator << (const Vec4ui& b) const;
    NFE_FORCE_INLINE const Vec4ui operator >> (const Vec4ui& b) const;
    NFE_FORCE_INLINE Vec4ui& operator <<= (const Vec4ui& b);
    NFE_FORCE_INLINE Vec4ui& operator >>= (const Vec4ui& b);
    NFE_FORCE_INLINE const Vec4ui operator << (uint32 b) const;
    NFE_FORCE_INLINE const Vec4ui operator >> (uint32 b) const;
    NFE_FORCE_INLINE Vec4ui& operator <<= (uint32 b);
    NFE_FORCE_INLINE Vec4ui& operator >>= (uint32 b);

    // For each vector component, copy value from "a" if "sel" is "false", or from "b" otherwise
    NFE_FORCE_INLINE static const Vec4ui Select(const Vec4ui& a, const Vec4ui& b, const VecBool4i& sel);

    NFE_FORCE_INLINE const VecBool4i operator == (const Vec4ui& b) const;
    NFE_FORCE_INLINE const VecBool4i operator < (const Vec4ui& b) const;
    NFE_FORCE_INLINE const VecBool4i operator <= (const Vec4ui& b) const;
    NFE_FORCE_INLINE const VecBool4i operator > (const Vec4ui& b) const;
    NFE_FORCE_INLINE const VecBool4i operator >= (const Vec4ui& b) const;
    NFE_FORCE_INLINE const VecBool4i operator != (const Vec4ui& b) const;

    NFE_FORCE_INLINE static const Vec4ui Min(const Vec4ui& a, const Vec4ui& b);
    NFE_FORCE_INLINE static const Vec4ui Max(const Vec4ui& a, const Vec4ui& b);

    NFE_FORCE_INLINE const Vec4ui Clamped(const Vec4ui& min, const Vec4ui& max) const;

    // convert from float vector to integer vector (with rounding)
    NFE_FORCE_INLINE static const Vec4ui Convert(const Vec4f& v);

    // convert from float vector to integer vector (with truncation towards zero)
    NFE_FORCE_INLINE static const Vec4ui TruncateAndConvert(const Vec4f& v);

    // convert to float vector
    NFE_FORCE_INLINE const Vec4f ConvertToVec4f() const;

    // cast from float vector (preserve bits)
    NFE_FORCE_INLINE static const Vec4ui Cast(const Vec4f& v);

    // convert to float vector
    NFE_FORCE_INLINE const Vec4f AsVec4f() const;

    // Rearrange vector elements
    template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
    NFE_FORCE_INLINE const Vec4ui Swizzle() const;
};

} // namespace Math
} // namespace NFE


#ifdef NFE_USE_SSE
#include "Vec4iImplSSE.hpp"
#else
#include "Vec4iImplNaive.hpp"
#endif
