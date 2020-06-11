#pragma once

#include "Math.hpp"
#include "Vec16f.hpp"

namespace NFE {
namespace Math {

/**
 * Sixteen 2D float vectors.
 */
class NFE_ALIGN(64) Vec2x16f
{
public:
    Vec16f x;
    Vec16f y;

    NFE_FORCE_INLINE Vec2x16f() = default;
    NFE_FORCE_INLINE Vec2x16f(const Vec2x16f&) = default;
    NFE_FORCE_INLINE Vec2x16f& operator = (const Vec2x16f&) = default;

    NFE_FORCE_INLINE static const Vec2x16f Zero()
    {
        return { Vec16f::Zero(), Vec16f::Zero() };
    }

    NFE_FORCE_INLINE static const Vec2x16f One()
    {
        return { Vec16f(1.0f), Vec16f(1.0f) };
    }

    NFE_FORCE_INLINE explicit Vec2x16f(const float scalar)
        : x(scalar), y(scalar)
    { }

    NFE_FORCE_INLINE Vec2x16f(const Vec16f& x, const Vec16f& y)
        : x(x), y(y)
    { }

    // splat single 3D vector
    NFE_FORCE_INLINE explicit Vec2x16f(const Vec4f& v)
    {
        x = Vec16f{ v.x };
        y = Vec16f{ v.y };
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE const Vec2x16f operator + (const Vec2x16f& rhs) const
    {
        return { x + rhs.x, y + rhs.y };
    }

    NFE_FORCE_INLINE const Vec2x16f operator - (const Vec2x16f& rhs) const
    {
        return { x - rhs.x, y - rhs.y };
    }

    NFE_FORCE_INLINE const Vec2x16f operator * (const Vec2x16f& rhs) const
    {
        return { x * rhs.x, y * rhs.y };
    }

    NFE_FORCE_INLINE const Vec2x16f operator * (const Vec16f& rhs) const
    {
        return { x * rhs, y * rhs };
    }

    NFE_FORCE_INLINE const Vec2x16f operator / (const Vec2x16f& rhs) const
    {
        return { x / rhs.x, y / rhs.y };
    }

    NFE_FORCE_INLINE const Vec2x16f operator * (const float rhs) const
    {
        return { x * rhs, y * rhs };
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE Vec2x16f& operator += (const Vec2x16f& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    NFE_FORCE_INLINE Vec2x16f& operator -= (const Vec2x16f& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    NFE_FORCE_INLINE Vec2x16f& operator *= (const Vec2x16f& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }

    NFE_FORCE_INLINE Vec2x16f& operator /= (const Vec2x16f& rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }

    NFE_FORCE_INLINE Vec2x16f& operator *= (const float rhs)
    {
        x *= rhs;
        y *= rhs;
        return *this;
    }

    NFE_FORCE_INLINE static const Vec2x16f FastReciprocal(const Vec2x16f& v)
    {
        return
        {
            Vec16f::FastReciprocal(v.x),
            Vec16f::FastReciprocal(v.y)
        };
    }

    NFE_FORCE_INLINE static const Vec2x16f MulAndAdd(const Vec2x16f& a, const Vec2x16f& b, const Vec2x16f& c)
    {
        return
        {
            Vec16f::MulAndAdd(a.x, b.x, c.x),
            Vec16f::MulAndAdd(a.y, b.y, c.y)
        };
    }

    NFE_FORCE_INLINE static const Vec2x16f MulAndSub(const Vec2x16f& a, const Vec2x16f& b, const Vec2x16f& c)
    {
        return
        {
            Vec16f::MulAndSub(a.x, b.x, c.x),
            Vec16f::MulAndSub(a.y, b.y, c.y)
        };
    }

    NFE_FORCE_INLINE static const Vec2x16f NegMulAndAdd(const Vec2x16f& a, const Vec2x16f& b, const Vec2x16f& c)
    {
        return
        {
            Vec16f::NegMulAndAdd(a.x, b.x, c.x),
            Vec16f::NegMulAndAdd(a.y, b.y, c.y)
        };
    }

    NFE_FORCE_INLINE static const Vec2x16f NegMulAndSub(const Vec2x16f& a, const Vec2x16f& b, const Vec2x16f& c)
    {
        return
        {
            Vec16f::NegMulAndSub(a.x, b.x, c.x),
            Vec16f::NegMulAndSub(a.y, b.y, c.y)
        };
    }

    //////////////////////////////////////////////////////////////////////////

    // dot product
    NFE_FORCE_INLINE static const Vec16f Dot(const Vec2x16f& a, const Vec2x16f& b)
    {
        // return a.x * b.x + a.y * b.y
        return Vec16f::MulAndAdd(a.x, b.x, a.y * b.y);
    }
    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE static const Vec2x16f Min(const Vec2x16f& a, const Vec2x16f& b)
    {
        return { Vec16f::Min(a.x, b.x), Vec16f::Min(a.y, b.y) };
    }

    NFE_FORCE_INLINE static const Vec2x16f Max(const Vec2x16f& a, const Vec2x16f& b)
    {
        return { Vec16f::Max(a.x, b.x), Vec16f::Max(a.y, b.y) };
    }

};

static_assert(sizeof(Vec2x16f) == sizeof(float) * 2 * 16, "Invalid sizeof Vec2x16f");

NFE_FORCE_INLINE const Vec2x16f operator * (const float lhs, const Vec2x16f rhs)
{
    return { lhs * rhs.x, lhs * rhs.y };
}


} // namespace Math
} // namespace NFE
