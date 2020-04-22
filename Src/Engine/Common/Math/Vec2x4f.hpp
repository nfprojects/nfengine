#pragma once

#include "Math.hpp"
#include "Vec4f.hpp"


namespace NFE {
namespace Math {

/**
 * Four 2D vectors.
 */
class NFE_ALIGN(32) Vec2x4f
{
public:
    Vec4f x;
    Vec4f y;

    NFE_FORCE_INLINE Vec2x4f() = default;
    NFE_FORCE_INLINE Vec2x4f(const Vec2x4f&) = default;
    NFE_FORCE_INLINE Vec2x4f& operator = (const Vec2x4f&) = default;

    NFE_FORCE_INLINE static const Vec2x4f Zero()
    {
        return { Vec4f::Zero(), Vec4f::Zero() };
    }

    NFE_FORCE_INLINE static const Vec2x4f One()
    {
        return { Vec4f(1.0f), Vec4f(1.0f) };
    }

    NFE_FORCE_INLINE explicit Vec2x4f(const float scalar)
        : x(scalar), y(scalar)
    { }

    NFE_FORCE_INLINE Vec2x4f(const Vec4f& x, const Vec4f& y)
        : x(x), y(y)
    { }

    // splat single 2D vector
    NFE_FORCE_INLINE explicit Vec2x4f(const Vec4f& v)
    {
        x = Vec4f(v.x);
        y = Vec4f(v.y);
    }

    // build from four 2D vectors
    NFE_FORCE_INLINE Vec2x4f(
        const Vec4f& v0,
        const Vec4f& v1,
        const Vec4f& v2,
        const Vec4f& v3)
    {
#ifdef NFE_USE_SSE
        __m128 tmp0 = _mm_shuffle_ps(v0, v1, 0x44);
        __m128 tmp2 = _mm_shuffle_ps(v0, v1, 0xEE);
        __m128 tmp1 = _mm_shuffle_ps(v2, v3, 0x44);
        __m128 tmp3 = _mm_shuffle_ps(v2, v3, 0xEE);
        x = _mm_shuffle_ps(tmp0, tmp1, 0x88);
        y = _mm_shuffle_ps(tmp0, tmp1, 0xDD);
#else // NFE_USE_SSE
        x = Vec4f{ v0.x, v1.x, v2.x, v3.x };
        y = Vec4f{ v0.y, v1.y, v2.y, v3.y };
#endif // NFE_USE_SSE
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE const Vec2x4f operator + (const Vec2x4f& rhs) const
    {
        return { x + rhs.x, y + rhs.y };
    }

    NFE_FORCE_INLINE const Vec2x4f operator - (const Vec2x4f& rhs) const
    {
        return { x - rhs.x, y - rhs.y };
    }

    NFE_FORCE_INLINE const Vec2x4f operator * (const Vec2x4f& rhs) const
    {
        return { x * rhs.x, y * rhs.y };
    }

    NFE_FORCE_INLINE const Vec2x4f operator * (const Vec4f& rhs) const
    {
        return { x * rhs, y * rhs };
    }

    NFE_FORCE_INLINE const Vec2x4f operator / (const Vec2x4f& rhs) const
    {
        return { x / rhs.x, y / rhs.y };
    }

    NFE_FORCE_INLINE const Vec2x4f operator * (const float rhs) const
    {
        return { x * rhs, y * rhs };
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE Vec2x4f& operator += (const Vec2x4f& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    NFE_FORCE_INLINE Vec2x4f& operator -= (const Vec2x4f& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    NFE_FORCE_INLINE Vec2x4f& operator *= (const Vec2x4f& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }

    NFE_FORCE_INLINE Vec2x4f& operator /= (const Vec2x4f& rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }

    NFE_FORCE_INLINE Vec2x4f& operator *= (const float rhs)
    {
        x *= rhs;
        y *= rhs;
        return *this;
    }

    NFE_FORCE_INLINE static const Vec2x4f FastReciprocal(const Vec2x4f& v)
    {
        return
        {
            Vec4f::FastReciprocal(v.x),
            Vec4f::FastReciprocal(v.y)
        };
    }

    NFE_FORCE_INLINE static const Vec2x4f MulAndAdd(const Vec2x4f& a, const Vec2x4f& b, const Vec2x4f& c)
    {
        return
        {
            Vec4f::MulAndAdd(a.x, b.x, c.x),
            Vec4f::MulAndAdd(a.y, b.y, c.y)
        };
    }

    NFE_FORCE_INLINE static const Vec2x4f MulAndSub(const Vec2x4f& a, const Vec2x4f& b, const Vec2x4f& c)
    {
        return
        {
            Vec4f::MulAndSub(a.x, b.x, c.x),
            Vec4f::MulAndSub(a.y, b.y, c.y)
        };
    }

    NFE_FORCE_INLINE static const Vec2x4f NegMulAndAdd(const Vec2x4f& a, const Vec2x4f& b, const Vec2x4f& c)
    {
        return
        {
            Vec4f::NegMulAndAdd(a.x, b.x, c.x),
            Vec4f::NegMulAndAdd(a.y, b.y, c.y)
        };
    }

    NFE_FORCE_INLINE static const Vec2x4f NegMulAndSub(const Vec2x4f& a, const Vec2x4f& b, const Vec2x4f& c)
    {
        return
        {
            Vec4f::NegMulAndSub(a.x, b.x, c.x),
            Vec4f::NegMulAndSub(a.y, b.y, c.y)
        };
    }

    //////////////////////////////////////////////////////////////////////////

    // dot product
    NFE_FORCE_INLINE static const Vec4f Dot(const Vec2x4f& a, const Vec2x4f& b)
    {
        // return a.x * b.x + a.y * b.y
        return Vec4f::MulAndAdd(a.x, b.x, a.y * b.y);
    }
    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE static const Vec2x4f Min(const Vec2x4f& a, const Vec2x4f& b)
    {
        return { Vec4f::Min(a.x, b.x), Vec4f::Min(a.y, b.y) };
    }

    NFE_FORCE_INLINE static const Vec2x4f Max(const Vec2x4f& a, const Vec2x4f& b)
    {
        return { Vec4f::Max(a.x, b.x), Vec4f::Max(a.y, b.y) };
    }

};


NFE_FORCE_INLINE const Vec2x4f operator * (const float lhs, const Vec2x4f rhs)
{
    return { lhs * rhs.x, lhs * rhs.y };
}


} // namespace Math
} // namespace NFE
