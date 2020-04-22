#pragma once

#include "Math.hpp"
#include "Vec4f.hpp"

namespace NFE {
namespace Math {

/**
 * Four 3D vectors
 */
class NFE_ALIGN(32) Vec3x4f
{
public:
    Vec4f x;
    Vec4f y;
    Vec4f z;

    NFE_FORCE_INLINE Vec3x4f() = default;
    NFE_FORCE_INLINE Vec3x4f(const Vec3x4f&) = default;
    NFE_FORCE_INLINE Vec3x4f& operator = (const Vec3x4f&) = default;

    NFE_FORCE_INLINE static const Vec3x4f Zero()
    {
        return { Vec4f::Zero(), Vec4f::Zero(), Vec4f::Zero() };
    }

    NFE_FORCE_INLINE static const Vec3x4f One()
    {
        return { Vec4f(1.0f), Vec4f(1.0f), Vec4f(1.0f) };
    }

    NFE_FORCE_INLINE Vec3x4f(const Vec4f& x, const Vec4f& y, const Vec4f& z)
        : x(x), y(y), z(z)
    {}

    // splat value to all the components
    NFE_FORCE_INLINE static const Vec3x4f FromScalar(const Vec4f& s)
    {
        return Vec3x4f{ s, s, s };
    }

    // splat single 3D vector
    NFE_FORCE_INLINE explicit Vec3x4f(const Vec4f& v)
    {
        x = Vec4f{ v.x };
        y = Vec4f{ v.y };
        z = Vec4f{ v.z };
    }

    // splat single scalar to all components an elements
    NFE_FORCE_INLINE explicit Vec3x4f(const float f)
        : x(f) , y(f) , z(f)
    {}

    // splat single 3D vector
    NFE_FORCE_INLINE explicit Vec3x4f(const Vec3f& v)
        : x(v.x), y(v.y), z(v.z)
    {}

    // build from eight 3D vectors
    NFE_FORCE_INLINE Vec3x4f(const Vec4f& v0, const Vec4f& v1, const Vec4f& v2, const Vec4f& v3)
    {
#ifdef NFE_USE_SSE
        const __m128 t0 = _mm_unpacklo_ps(v0, v1);
        const __m128 t1 = _mm_unpacklo_ps(v2, v3);
        const __m128 t2 = _mm_unpackhi_ps(v0, v1);
        const __m128 t3 = _mm_unpackhi_ps(v2, v3);
        x = _mm_movelh_ps(t0, t1);
        y = _mm_movehl_ps(t1, t0);
        z = _mm_movelh_ps(t2, t3);
#else // NFE_USE_SSE
        x = Vec4f{ v0.x, v1.x, v2.x, v3.x };
        y = Vec4f{ v0.y, v1.y, v2.y, v3.y };
        z = Vec4f{ v0.z, v1.z, v2.z, v3.z };
#endif // NFE_USE_SSE
    }

    // unpack to 8x Vec4f
    NFE_FORCE_INLINE void Unpack(Vec4f output[4]) const
    {
#ifdef NFE_USE_SSE
        const __m128 w = _mm_setzero_ps();
        const __m128 t0 = _mm_unpacklo_ps(x, y);
        const __m128 t1 = _mm_unpacklo_ps(z, w);
        const __m128 t2 = _mm_unpackhi_ps(x, y);
        const __m128 t3 = _mm_unpackhi_ps(z, w);
        output[0] =_mm_movelh_ps(t0, t1);
        output[1] =_mm_movehl_ps(t1, t0);
        output[2] =_mm_movelh_ps(t2, t3);
        output[3] =_mm_movehl_ps(t3, t2);
#else // NFE_USE_SSE
        for (uint32 i = 0; i < 4; ++i)
        {
            output[i] = Vec4f(x[i], y[i], z[i]);
        }
#endif // NFE_USE_SSE
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE const Vec3x4f operator - () const
    {
        return { -x, -y, -z };
    }

    NFE_FORCE_INLINE const Vec3x4f operator + (const Vec3x4f& rhs) const
    {
        return { x + rhs.x, y + rhs.y, z + rhs.z };
    }

    NFE_FORCE_INLINE const Vec3x4f operator - (const Vec3x4f& rhs) const
    {
        return { x - rhs.x, y - rhs.y, z - rhs.z };
    }

    NFE_FORCE_INLINE const Vec3x4f operator * (const Vec3x4f& rhs) const
    {
        return { x * rhs.x, y * rhs.y,z * rhs.z };
    }

    NFE_FORCE_INLINE const Vec3x4f operator * (const Vec4f& rhs) const
    {
        return { x * rhs, y * rhs, z * rhs };
    }

    NFE_FORCE_INLINE const Vec3x4f operator / (const Vec3x4f& rhs) const
    {
        return { x / rhs.x, y / rhs.y, z / rhs.z };
    }

    NFE_FORCE_INLINE const Vec3x4f operator * (const float rhs) const
    {
        return { x * rhs, y * rhs, z * rhs };
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE Vec3x4f& operator += (const Vec3x4f& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    NFE_FORCE_INLINE Vec3x4f& operator -= (const Vec3x4f& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    NFE_FORCE_INLINE Vec3x4f& operator *= (const Vec3x4f& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }

    NFE_FORCE_INLINE Vec3x4f& operator /= (const Vec3x4f& rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        return *this;
    }

    NFE_FORCE_INLINE Vec3x4f& operator *= (const Vec4f& rhs)
    {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }

    NFE_FORCE_INLINE Vec3x4f& operator *= (const float rhs)
    {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }

    NFE_FORCE_INLINE static const Vec3x4f Reciprocal(const Vec3x4f& v)
    {
        return
        {
            Vec4f::Reciprocal(v.x),
            Vec4f::Reciprocal(v.y),
            Vec4f::Reciprocal(v.z)
        };
    }

    NFE_FORCE_INLINE static const Vec3x4f FastReciprocal(const Vec3x4f& v)
    {
        return
        {
            Vec4f::FastReciprocal(v.x),
            Vec4f::FastReciprocal(v.y),
            Vec4f::FastReciprocal(v.z)
        };
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE static const Vec3x4f MulAndAdd(const Vec3x4f& a, const Vec3x4f& b, const Vec3x4f& c)
    {
        return { Vec4f::MulAndAdd(a.x, b.x, c.x), Vec4f::MulAndAdd(a.y, b.y, c.y), Vec4f::MulAndAdd(a.z, b.z, c.z) };
    }

    NFE_FORCE_INLINE static const Vec3x4f MulAndSub(const Vec3x4f& a, const Vec3x4f& b, const Vec3x4f& c)
    {
        return { Vec4f::MulAndSub(a.x, b.x, c.x), Vec4f::MulAndSub(a.y, b.y, c.y), Vec4f::MulAndSub(a.z, b.z, c.z) };
    }

    NFE_FORCE_INLINE static const Vec3x4f NegMulAndAdd(const Vec3x4f& a, const Vec3x4f& b, const Vec3x4f& c)
    {
        return { Vec4f::NegMulAndAdd(a.x, b.x, c.x), Vec4f::NegMulAndAdd(a.y, b.y, c.y), Vec4f::NegMulAndAdd(a.z, b.z, c.z) };
    }

    NFE_FORCE_INLINE static const Vec3x4f NegMulAndSub(const Vec3x4f& a, const Vec3x4f& b, const Vec3x4f& c)
    {
        return { Vec4f::NegMulAndSub(a.x, b.x, c.x), Vec4f::NegMulAndSub(a.y, b.y, c.y), Vec4f::NegMulAndSub(a.z, b.z, c.z) };
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE static const Vec3x4f MulAndAdd(const Vec3x4f& a, const Vec4f& b, const Vec3x4f& c)
    {
        return { Vec4f::MulAndAdd(a.x, b, c.x), Vec4f::MulAndAdd(a.y, b, c.y), Vec4f::MulAndAdd(a.z, b, c.z) };
    }

    NFE_FORCE_INLINE static const Vec3x4f MulAndSub(const Vec3x4f& a, const Vec4f& b, const Vec3x4f& c)
    {
        return { Vec4f::MulAndSub(a.x, b, c.x), Vec4f::MulAndSub(a.y, b, c.y), Vec4f::MulAndSub(a.z, b, c.z) };
    }

    NFE_FORCE_INLINE static const Vec3x4f NegMulAndAdd(const Vec3x4f& a, const Vec4f& b, const Vec3x4f& c)
    {
        return { Vec4f::NegMulAndAdd(a.x, b, c.x), Vec4f::NegMulAndAdd(a.y, b, c.y), Vec4f::NegMulAndAdd(a.z, b, c.z) };
    }

    NFE_FORCE_INLINE static const Vec3x4f NegMulAndSub(const Vec3x4f& a, const Vec4f& b, const Vec3x4f& c)
    {
        return { Vec4f::NegMulAndSub(a.x, b, c.x), Vec4f::NegMulAndSub(a.y, b, c.y), Vec4f::NegMulAndSub(a.z, b, c.z) };
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE static const Vec3x4f MulAndAdd(const Vec3x4f& a, const Vec3x4f& b, const Vec4f& c)
    {
        return { Vec4f::MulAndAdd(a.x, b.x, c), Vec4f::MulAndAdd(a.y, b.y, c), Vec4f::MulAndAdd(a.z, b.z, c) };
    }

    NFE_FORCE_INLINE static const Vec3x4f MulAndSub(const Vec3x4f& a, const Vec3x4f& b, const Vec4f& c)
    {
        return { Vec4f::MulAndSub(a.x, b.x, c), Vec4f::MulAndSub(a.y, b.y, c), Vec4f::MulAndSub(a.z, b.z, c) };
    }

    NFE_FORCE_INLINE static const Vec3x4f NegMulAndAdd(const Vec3x4f& a, const Vec3x4f& b, const Vec4f& c)
    {
        return { Vec4f::NegMulAndAdd(a.x, b.x, c), Vec4f::NegMulAndAdd(a.y, b.y, c), Vec4f::NegMulAndAdd(a.z, b.z, c) };
    }

    NFE_FORCE_INLINE static const Vec3x4f NegMulAndSub(const Vec3x4f& a, const Vec3x4f& b, const Vec4f& c)
    {
        return { Vec4f::NegMulAndSub(a.x, b.x, c), Vec4f::NegMulAndSub(a.y, b.y, c), Vec4f::NegMulAndSub(a.z, b.z, c) };
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE static const Vec3x4f MulAndAdd(const Vec3x4f& a, const Vec4f& b, const Vec4f& c)
    {
        return { Vec4f::MulAndAdd(a.x, b, c), Vec4f::MulAndAdd(a.y, b, c), Vec4f::MulAndAdd(a.z, b, c) };
    }

    NFE_FORCE_INLINE static const Vec3x4f MulAndSub(const Vec3x4f& a, const Vec4f& b, const Vec4f& c)
    {
        return { Vec4f::MulAndSub(a.x, b, c), Vec4f::MulAndSub(a.y, b, c), Vec4f::MulAndSub(a.z, b, c) };
    }

    NFE_FORCE_INLINE static const Vec3x4f NegMulAndAdd(const Vec3x4f& a, const Vec4f& b, const Vec4f& c)
    {
        return { Vec4f::NegMulAndAdd(a.x, b, c), Vec4f::NegMulAndAdd(a.y, b, c), Vec4f::NegMulAndAdd(a.z, b, c) };
    }

    NFE_FORCE_INLINE static const Vec3x4f NegMulAndSub(const Vec3x4f& a, const Vec4f& b, const Vec4f& c)
    {
        return { Vec4f::NegMulAndSub(a.x, b, c), Vec4f::NegMulAndSub(a.y, b, c), Vec4f::NegMulAndSub(a.z, b, c) };
    }

    //////////////////////////////////////////////////////////////////////////

    // 3D dot product
    NFE_FORCE_INLINE static const Vec4f Dot(const Vec3x4f& a, const Vec3x4f& b)
    {
        // return a.x * b.x + a.y * b.y + a.z * b.z;
        return Vec4f::MulAndAdd(a.x, b.x, Vec4f::MulAndAdd(a.y, b.y, a.z * b.z));
    }

    // 3D cross product
    NFE_FORCE_INLINE static const Vec3x4f Cross(const Vec3x4f& a, const Vec3x4f& b)
    {
        return {
            Vec4f::NegMulAndAdd(a.z, b.y, a.y * b.z),
            Vec4f::NegMulAndAdd(a.x, b.z, a.z * b.x),
            Vec4f::NegMulAndAdd(a.y, b.x, a.x * b.y)
        };
    }

    NFE_FORCE_INLINE const Vec4f SqrLength() const
    {
        return Dot(*this, *this);
    }

    NFE_FORCE_INLINE const Vec3x4f Normalized() const
    {
        const Vec4f invLength = Vec4f::Reciprocal(Vec4f::Sqrt(SqrLength()));
        return (*this) * invLength;
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE static const Vec3x4f Min(const Vec3x4f& a, const Vec3x4f& b)
    {
        return { Vec4f::Min(a.x, b.x), Vec4f::Min(a.y, b.y), Vec4f::Min(a.z, b.z) };
    }

    NFE_FORCE_INLINE static const Vec3x4f Max(const Vec3x4f& a, const Vec3x4f& b)
    {
        return { Vec4f::Max(a.x, b.x), Vec4f::Max(a.y, b.y), Vec4f::Max(a.z, b.z) };
    }

};


} // namespace Math
} // namespace NFE
