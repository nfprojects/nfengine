#pragma once

#include "Math.hpp"
#include "VectorBool4.hpp"

namespace NFE {
namespace Math {

struct Vector4;

/**
 * 4-element signed integer SIMD vector.
 */
struct NFE_ALIGN(16) VectorInt4
{
    union
    {
        int32 i[4];
        int64 i64[2];


#ifdef NFE_USE_SSE
        __m128i v;
#endif // NFE_USE_SSE

        struct
        {
            int32 x;
            int32 y;
            int32 z;
            int32 w;
        };
    };

    // constructors
    NFE_FORCE_INLINE VectorInt4() = default;
    NFE_FORCE_INLINE static const VectorInt4 Zero();
    NFE_FORCE_INLINE VectorInt4(const VectorInt4& other);
    NFE_FORCE_INLINE VectorInt4(const VectorBool4& other);
    NFE_FORCE_INLINE explicit VectorInt4(const int32 scalar);
    NFE_FORCE_INLINE explicit VectorInt4(const uint32 scalar);
    NFE_FORCE_INLINE VectorInt4(const int32 x, const int32 y, const int32 z, const int32 w);

#ifdef NFE_USE_SSE
    NFE_FORCE_INLINE VectorInt4(const __m128i& m);
    NFE_FORCE_INLINE operator __m128i() const { return v; }
#endif // NFE_USE_SSE

    NFE_FORCE_INLINE int32 operator[] (const uint32 index) const { return i[index]; }

    // bitwise logic operations
    NFE_FORCE_INLINE const VectorInt4 operator & (const VectorInt4& b) const;
    NFE_FORCE_INLINE const VectorInt4 operator | (const VectorInt4& b) const;
    NFE_FORCE_INLINE const VectorInt4 operator ^ (const VectorInt4& b) const;
    NFE_FORCE_INLINE VectorInt4& operator &= (const VectorInt4& b);
    NFE_FORCE_INLINE VectorInt4& operator |= (const VectorInt4& b);
    NFE_FORCE_INLINE VectorInt4& operator ^= (const VectorInt4& b);
    NFE_FORCE_INLINE static const VectorInt4 AndNot(const VectorInt4& a, const VectorInt4& b);

    // simple arithmetics
    NFE_FORCE_INLINE const VectorInt4 operator - () const;
    NFE_FORCE_INLINE const VectorInt4 operator + (const VectorInt4& b) const;
    NFE_FORCE_INLINE const VectorInt4 operator - (const VectorInt4& b) const;
    NFE_FORCE_INLINE const VectorInt4 operator * (const VectorInt4& b) const;
    NFE_FORCE_INLINE VectorInt4& operator += (const VectorInt4& b);
    NFE_FORCE_INLINE VectorInt4& operator -= (const VectorInt4& b);
    NFE_FORCE_INLINE VectorInt4& operator *= (const VectorInt4& b);
    NFE_FORCE_INLINE const VectorInt4 operator + (int32 b) const;
    NFE_FORCE_INLINE const VectorInt4 operator - (int32 b) const;
    NFE_FORCE_INLINE const VectorInt4 operator * (int32 b) const;
    NFE_FORCE_INLINE VectorInt4& operator += (int32 b);
    NFE_FORCE_INLINE VectorInt4& operator -= (int32 b);
    NFE_FORCE_INLINE VectorInt4& operator *= (int32 b);

    // bit shifting
    NFE_FORCE_INLINE const VectorInt4 operator << (const VectorInt4& b) const;
    NFE_FORCE_INLINE const VectorInt4 operator >> (const VectorInt4& b) const;
    NFE_FORCE_INLINE VectorInt4& operator <<= (const VectorInt4& b);
    NFE_FORCE_INLINE VectorInt4& operator >>= (const VectorInt4& b);
    NFE_FORCE_INLINE const VectorInt4 operator << (int32 b) const;
    NFE_FORCE_INLINE const VectorInt4 operator >> (int32 b) const;
    NFE_FORCE_INLINE VectorInt4& operator <<= (int32 b);
    NFE_FORCE_INLINE VectorInt4& operator >>= (int32 b);

    // For each vector component, copy value from "a" if "sel" is "false", or from "b" otherwise
    NFE_FORCE_INLINE static const VectorInt4 Select(const VectorInt4& a, const VectorInt4& b, const VectorBool4& sel);

    NFE_FORCE_INLINE const VectorBool4 operator == (const VectorInt4& b) const;
    NFE_FORCE_INLINE const VectorBool4 operator < (const VectorInt4& b) const;
    NFE_FORCE_INLINE const VectorBool4 operator <= (const VectorInt4& b) const;
    NFE_FORCE_INLINE const VectorBool4 operator > (const VectorInt4& b) const;
    NFE_FORCE_INLINE const VectorBool4 operator >= (const VectorInt4& b) const;
    NFE_FORCE_INLINE const VectorBool4 operator != (const VectorInt4& b) const;

    NFE_FORCE_INLINE static const VectorInt4 Min(const VectorInt4& a, const VectorInt4& b);
    NFE_FORCE_INLINE static const VectorInt4 Max(const VectorInt4& a, const VectorInt4& b);

    NFE_FORCE_INLINE const VectorInt4 Clamped(const VectorInt4& min, const VectorInt4& max) const;

    // convert from float vector to integer vector (with rounding)
    NFE_FORCE_INLINE static const VectorInt4 Convert(const Vector4& v);

    // convert from float vector to integer vector (with truncation towards zero)
    NFE_FORCE_INLINE static const VectorInt4 TruncateAndConvert(const Vector4& v);

    // convert to float vector
    NFE_FORCE_INLINE const Vector4 ConvertToFloat() const;

    // cast from float vector (preserve bits)
    NFE_FORCE_INLINE static const VectorInt4 Cast(const Vector4& v);

    // convert to float vector
    NFE_FORCE_INLINE const Vector4 CastToFloat() const;

    // Rearrange vector elements
    template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
    NFE_FORCE_INLINE const VectorInt4 Swizzle() const;
};

} // namespace Math
} // namespace NFE


#ifdef NFE_USE_SSE
#include "VectorInt4ImplSSE.hpp"
#else
#include "VectorInt4ImplNaive.hpp"
#endif
