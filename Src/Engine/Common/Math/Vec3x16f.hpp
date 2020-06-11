#pragma once

#include "Math.hpp"
#include "Vec16f.hpp"

namespace NFE {
namespace Math {

/**
 * Sixteen 3D float vectors.
 */
class NFE_ALIGN(64) Vec3x16f
{
public:
    Vec16f x;
    Vec16f y;
    Vec16f z;

    NFE_FORCE_INLINE Vec3x16f() = default;
    NFE_FORCE_INLINE Vec3x16f(const Vec3x16f&) = default;
    NFE_FORCE_INLINE Vec3x16f& operator = (const Vec3x16f&) = default;

    NFE_FORCE_INLINE static const Vec3x16f Zero()
    {
        return { Vec16f::Zero(), Vec16f::Zero(), Vec16f::Zero() };
    }

    NFE_FORCE_INLINE static const Vec3x16f One()
    {
        return { Vec16f(1.0f), Vec16f(1.0f), Vec16f(1.0f) };
    }

    NFE_FORCE_INLINE Vec3x16f(const Vec16f& x, const Vec16f& y, const Vec16f& z)
        : x(x), y(y), z(z)
    {}

    // splat value to all the components
    NFE_FORCE_INLINE static const Vec3x16f FromScalar(const Vec16f& s)
    {
        return Vec3x16f{ s, s, s };
    }

    // splat single 3D vector
    NFE_FORCE_INLINE explicit Vec3x16f(const Vec4f& v)
    {
        x = Vec16f{ v.x };
        y = Vec16f{ v.y };
        z = Vec16f{ v.z };
    }

    // splat single scalar to all components an elements
    NFE_FORCE_INLINE explicit Vec3x16f(const float f)
        : x(f) , y(f) , z(f)
    {}

    // splat single 3D vector
    NFE_FORCE_INLINE explicit Vec3x16f(const Vec3f& v)
        : x(v.x), y(v.y), z(v.z)
    {}

    // unpack to 8x Vec4f
    NFE_FORCE_INLINE void Unpack(Vec4f out[16]) const
    {
#ifdef NFE_USE_AVX512

        __m512 w = _mm512_setzero_ps();

        __m512 t0, t1, t2, t3, t4, t5, t6, t7;
        __m512 r0, r1, r2, r3;

        t0 = _mm512_unpacklo_ps(x, y);
        t1 = _mm512_unpackhi_ps(x, y);
        t2 = _mm512_unpacklo_ps(z, w);
        t3 = _mm512_unpackhi_ps(z, w);

        r0 = _mm512_shuffle_ps(t0, t2, 0x44); // _mm512_unpacklo_ps
        r1 = _mm512_shuffle_ps(t0, t2, 0xee); // _mm512_unpackhi_ps
        r2 = _mm512_shuffle_ps(t1, t3, 0x44); // _mm512_unpacklo_ps
        r3 = _mm512_shuffle_ps(t1, t3, 0xee); // _mm512_unpackhi_ps

        t0 = _mm512_shuffle_f32x4(r0, r0, 0x88);
        t1 = _mm512_shuffle_f32x4(r1, r1, 0x88);
        t2 = _mm512_shuffle_f32x4(r2, r2, 0x88);
        t3 = _mm512_shuffle_f32x4(r3, r3, 0x88);
        t4 = _mm512_shuffle_f32x4(r0, r0, 0xdd);
        t5 = _mm512_shuffle_f32x4(r1, r1, 0xdd);
        t6 = _mm512_shuffle_f32x4(r2, r2, 0xdd);
        t7 = _mm512_shuffle_f32x4(r3, r3, 0xdd);

        out[ 0] = _mm512_castps512_ps128(_mm512_shuffle_f32x4(t0, t0, 0x88));
        out[ 1] = _mm512_castps512_ps128(_mm512_shuffle_f32x4(t1, t1, 0x88));
        out[ 2] = _mm512_castps512_ps128(_mm512_shuffle_f32x4(t2, t2, 0x88));
        out[ 3] = _mm512_castps512_ps128(_mm512_shuffle_f32x4(t3, t3, 0x88));
        out[ 4] = _mm512_castps512_ps128(_mm512_shuffle_f32x4(t4, t4, 0x88));
        out[ 5] = _mm512_castps512_ps128(_mm512_shuffle_f32x4(t5, t5, 0x88));
        out[ 6] = _mm512_castps512_ps128(_mm512_shuffle_f32x4(t6, t6, 0x88));
        out[ 7] = _mm512_castps512_ps128(_mm512_shuffle_f32x4(t7, t7, 0x88));
        out[ 8] = _mm512_castps512_ps128(_mm512_shuffle_f32x4(t0, t0, 0xdd));
        out[ 9] = _mm512_castps512_ps128(_mm512_shuffle_f32x4(t1, t1, 0xdd));
        out[10] = _mm512_castps512_ps128(_mm512_shuffle_f32x4(t2, t2, 0xdd));
        out[11] = _mm512_castps512_ps128(_mm512_shuffle_f32x4(t3, t3, 0xdd));
        out[12] = _mm512_castps512_ps128(_mm512_shuffle_f32x4(t4, t4, 0xdd));
        out[13] = _mm512_castps512_ps128(_mm512_shuffle_f32x4(t5, t5, 0xdd));
        out[14] = _mm512_castps512_ps128(_mm512_shuffle_f32x4(t6, t6, 0xdd));
        out[15] = _mm512_castps512_ps128(_mm512_shuffle_f32x4(t7, t7, 0xdd));

#else // !NFE_USE_AVX512

        for (uint32 i = 0; i < 16; ++i)
        {
            out[i] = Vec4f(x[i], y[i], z[i]);
        }

#endif // !NFE_USE_AVX512
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE const Vec3x16f operator - () const
    {
        return { -x, -y, -z };
    }

    NFE_FORCE_INLINE const Vec3x16f operator + (const Vec3x16f& rhs) const
    {
        return { x + rhs.x, y + rhs.y, z + rhs.z };
    }

    NFE_FORCE_INLINE const Vec3x16f operator - (const Vec3x16f& rhs) const
    {
        return { x - rhs.x, y - rhs.y, z - rhs.z };
    }

    NFE_FORCE_INLINE const Vec3x16f operator * (const Vec3x16f& rhs) const
    {
        return { x * rhs.x, y * rhs.y,z * rhs.z };
    }

    NFE_FORCE_INLINE const Vec3x16f operator * (const Vec16f& rhs) const
    {
        return { x * rhs, y * rhs, z * rhs };
    }

    NFE_FORCE_INLINE const Vec3x16f operator / (const Vec3x16f& rhs) const
    {
        return { x / rhs.x, y / rhs.y, z / rhs.z };
    }

    NFE_FORCE_INLINE const Vec3x16f operator * (const float rhs) const
    {
        return { x * rhs, y * rhs, z * rhs };
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE Vec3x16f& operator += (const Vec3x16f& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    NFE_FORCE_INLINE Vec3x16f& operator -= (const Vec3x16f& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    NFE_FORCE_INLINE Vec3x16f& operator *= (const Vec3x16f& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }

    NFE_FORCE_INLINE Vec3x16f& operator /= (const Vec3x16f& rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        return *this;
    }

    NFE_FORCE_INLINE Vec3x16f& operator *= (const Vec16f& rhs)
    {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }

    NFE_FORCE_INLINE Vec3x16f& operator *= (const float rhs)
    {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }

    NFE_FORCE_INLINE static const Vec3x16f Reciprocal(const Vec3x16f& v)
    {
        return
        {
            Vec16f::Reciprocal(v.x),
            Vec16f::Reciprocal(v.y),
            Vec16f::Reciprocal(v.z)
        };
    }

    NFE_FORCE_INLINE static const Vec3x16f FastReciprocal(const Vec3x16f& v)
    {
        return
        {
            Vec16f::FastReciprocal(v.x),
            Vec16f::FastReciprocal(v.y),
            Vec16f::FastReciprocal(v.z)
        };
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE static const Vec3x16f MulAndAdd(const Vec3x16f& a, const Vec3x16f& b, const Vec3x16f& c)
    {
        return { Vec16f::MulAndAdd(a.x, b.x, c.x), Vec16f::MulAndAdd(a.y, b.y, c.y), Vec16f::MulAndAdd(a.z, b.z, c.z) };
    }

    NFE_FORCE_INLINE static const Vec3x16f MulAndSub(const Vec3x16f& a, const Vec3x16f& b, const Vec3x16f& c)
    {
        return { Vec16f::MulAndSub(a.x, b.x, c.x), Vec16f::MulAndSub(a.y, b.y, c.y), Vec16f::MulAndSub(a.z, b.z, c.z) };
    }

    NFE_FORCE_INLINE static const Vec3x16f NegMulAndAdd(const Vec3x16f& a, const Vec3x16f& b, const Vec3x16f& c)
    {
        return { Vec16f::NegMulAndAdd(a.x, b.x, c.x), Vec16f::NegMulAndAdd(a.y, b.y, c.y), Vec16f::NegMulAndAdd(a.z, b.z, c.z) };
    }

    NFE_FORCE_INLINE static const Vec3x16f NegMulAndSub(const Vec3x16f& a, const Vec3x16f& b, const Vec3x16f& c)
    {
        return { Vec16f::NegMulAndSub(a.x, b.x, c.x), Vec16f::NegMulAndSub(a.y, b.y, c.y), Vec16f::NegMulAndSub(a.z, b.z, c.z) };
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE static const Vec3x16f MulAndAdd(const Vec3x16f& a, const Vec16f& b, const Vec3x16f& c)
    {
        return { Vec16f::MulAndAdd(a.x, b, c.x), Vec16f::MulAndAdd(a.y, b, c.y), Vec16f::MulAndAdd(a.z, b, c.z) };
    }

    NFE_FORCE_INLINE static const Vec3x16f MulAndSub(const Vec3x16f& a, const Vec16f& b, const Vec3x16f& c)
    {
        return { Vec16f::MulAndSub(a.x, b, c.x), Vec16f::MulAndSub(a.y, b, c.y), Vec16f::MulAndSub(a.z, b, c.z) };
    }

    NFE_FORCE_INLINE static const Vec3x16f NegMulAndAdd(const Vec3x16f& a, const Vec16f& b, const Vec3x16f& c)
    {
        return { Vec16f::NegMulAndAdd(a.x, b, c.x), Vec16f::NegMulAndAdd(a.y, b, c.y), Vec16f::NegMulAndAdd(a.z, b, c.z) };
    }

    NFE_FORCE_INLINE static const Vec3x16f NegMulAndSub(const Vec3x16f& a, const Vec16f& b, const Vec3x16f& c)
    {
        return { Vec16f::NegMulAndSub(a.x, b, c.x), Vec16f::NegMulAndSub(a.y, b, c.y), Vec16f::NegMulAndSub(a.z, b, c.z) };
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE static const Vec3x16f MulAndAdd(const Vec3x16f& a, const Vec3x16f& b, const Vec16f& c)
    {
        return { Vec16f::MulAndAdd(a.x, b.x, c), Vec16f::MulAndAdd(a.y, b.y, c), Vec16f::MulAndAdd(a.z, b.z, c) };
    }

    NFE_FORCE_INLINE static const Vec3x16f MulAndSub(const Vec3x16f& a, const Vec3x16f& b, const Vec16f& c)
    {
        return { Vec16f::MulAndSub(a.x, b.x, c), Vec16f::MulAndSub(a.y, b.y, c), Vec16f::MulAndSub(a.z, b.z, c) };
    }

    NFE_FORCE_INLINE static const Vec3x16f NegMulAndAdd(const Vec3x16f& a, const Vec3x16f& b, const Vec16f& c)
    {
        return { Vec16f::NegMulAndAdd(a.x, b.x, c), Vec16f::NegMulAndAdd(a.y, b.y, c), Vec16f::NegMulAndAdd(a.z, b.z, c) };
    }

    NFE_FORCE_INLINE static const Vec3x16f NegMulAndSub(const Vec3x16f& a, const Vec3x16f& b, const Vec16f& c)
    {
        return { Vec16f::NegMulAndSub(a.x, b.x, c), Vec16f::NegMulAndSub(a.y, b.y, c), Vec16f::NegMulAndSub(a.z, b.z, c) };
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE static const Vec3x16f MulAndAdd(const Vec3x16f& a, const Vec16f& b, const Vec16f& c)
    {
        return { Vec16f::MulAndAdd(a.x, b, c), Vec16f::MulAndAdd(a.y, b, c), Vec16f::MulAndAdd(a.z, b, c) };
    }

    NFE_FORCE_INLINE static const Vec3x16f MulAndSub(const Vec3x16f& a, const Vec16f& b, const Vec16f& c)
    {
        return { Vec16f::MulAndSub(a.x, b, c), Vec16f::MulAndSub(a.y, b, c), Vec16f::MulAndSub(a.z, b, c) };
    }

    NFE_FORCE_INLINE static const Vec3x16f NegMulAndAdd(const Vec3x16f& a, const Vec16f& b, const Vec16f& c)
    {
        return { Vec16f::NegMulAndAdd(a.x, b, c), Vec16f::NegMulAndAdd(a.y, b, c), Vec16f::NegMulAndAdd(a.z, b, c) };
    }

    NFE_FORCE_INLINE static const Vec3x16f NegMulAndSub(const Vec3x16f& a, const Vec16f& b, const Vec16f& c)
    {
        return { Vec16f::NegMulAndSub(a.x, b, c), Vec16f::NegMulAndSub(a.y, b, c), Vec16f::NegMulAndSub(a.z, b, c) };
    }

    //////////////////////////////////////////////////////////////////////////

    // 3D dot product
    NFE_FORCE_INLINE static const Vec16f Dot(const Vec3x16f& a, const Vec3x16f& b)
    {
        // return a.x * b.x + a.y * b.y + a.z * b.z;
        return Vec16f::MulAndAdd(a.x, b.x, Vec16f::MulAndAdd(a.y, b.y, a.z * b.z));
    }

    // 3D cross product
    NFE_FORCE_INLINE static const Vec3x16f Cross(const Vec3x16f& a, const Vec3x16f& b)
    {
        return {
            Vec16f::NegMulAndAdd(a.z, b.y, a.y * b.z),
            Vec16f::NegMulAndAdd(a.x, b.z, a.z * b.x),
            Vec16f::NegMulAndAdd(a.y, b.x, a.x * b.y)
        };
    }

    NFE_FORCE_INLINE const Vec16f SqrLength() const
    {
        return Dot(*this, *this);
    }

    NFE_FORCE_INLINE const Vec3x16f Normalized() const
    {
        const Vec16f invLength = Vec16f::Reciprocal(Vec16f::Sqrt(SqrLength()));
        return (*this) * invLength;
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE static const Vec3x16f Min(const Vec3x16f& a, const Vec3x16f& b)
    {
        return { Vec16f::Min(a.x, b.x), Vec16f::Min(a.y, b.y), Vec16f::Min(a.z, b.z) };
    }

    NFE_FORCE_INLINE static const Vec3x16f Max(const Vec3x16f& a, const Vec3x16f& b)
    {
        return { Vec16f::Max(a.x, b.x), Vec16f::Max(a.y, b.y), Vec16f::Max(a.z, b.z) };
    }
};

static_assert(sizeof(Vec3x16f) == sizeof(float) * 3 * 16, "Invalid sizeof Vec3x16f");

} // namespace Math
} // namespace NFE
