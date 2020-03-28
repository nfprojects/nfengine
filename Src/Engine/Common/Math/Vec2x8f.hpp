#pragma once

#include "Math.hpp"
#include "Vec8f.hpp"


namespace NFE {
namespace Math {

/**
 * Eight 2D vectors.
 */
class NFE_ALIGN(32) Vec2x8f
{
public:
    Vec8f x;
    Vec8f y;

    NFE_FORCE_INLINE Vec2x8f() = default;
    NFE_FORCE_INLINE Vec2x8f(const Vec2x8f&) = default;
    NFE_FORCE_INLINE Vec2x8f& operator = (const Vec2x8f&) = default;

    NFE_FORCE_INLINE static const Vec2x8f Zero()
    {
        return { Vec8f::Zero(), Vec8f::Zero() };
    }

    NFE_FORCE_INLINE static const Vec2x8f One()
    {
        return { Vec8f(1.0f), Vec8f(1.0f) };
    }

    NFE_FORCE_INLINE explicit Vec2x8f(const float scalar)
        : x(scalar), y(scalar)
    { }

    NFE_FORCE_INLINE Vec2x8f(const Vec8f& x, const Vec8f& y)
        : x(x), y(y)
    { }

    // splat single 3D vector
    NFE_FORCE_INLINE explicit Vec2x8f(const Vec4f& v)
    {
#ifdef NFE_USE_AVX
        const Vec8f temp(v, v); // copy "v" onto both AVX lanes
        x = _mm256_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 0, 0, 0));
        y = _mm256_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 1, 1, 1));
#else
        x = Vec8f{ v.x };
        y = Vec8f{ v.y };
#endif // NFE_USE_AVX
    }

    // build from eight 3D vectors
    NFE_FORCE_INLINE Vec2x8f(
        const Vec4f& v0,
        const Vec4f& v1,
        const Vec4f& v2,
        const Vec4f& v3,
        const Vec4f& v4,
        const Vec4f& v5,
        const Vec4f& v6,
        const Vec4f& v7)
    {
        // TODO this can be probably optimized somehow

        // What is going on here is basically converting this:
        //
        // v0 = [ v0.x, v0.y, v0.z, v0.w ]
        // v1 = [ v1.x, v1.y, v1.z, v1.w ]
        // v2 = [ v2.x, v2.y, v2.z, v2.w ]
        // v3 = [ v3.x, v3.y, v3.z, v3.w ]
        // v4 = [ v4.x, v4.y, v4.z, v4.w ]
        // v5 = [ v5.x, v5.y, v5.z, v5.w ]
        // v6 = [ v6.x, v6.y, v6.z, v6.w ]
        // v7 = [ v7.x, v7.y, v7.z, v7.w ]
        //
        // into this:
        //
        // x = [ v0.x, v1.x, v2.x, v3.x, v4.x, v5.x, v6.x, v7.x ]
        // y = [ v0.y, v1.y, v2.y, v3.y, v4.y, v5.y, v6.y, v7.y ]
        //
        // note that "z" and "w" component are dropped

#ifdef NFE_USE_AVX

        const __m256 t0 = _mm256_unpacklo_ps(Vec8f(v0), Vec8f(v1));
        const __m256 t2 = _mm256_unpacklo_ps(Vec8f(v2), Vec8f(v3));
        const __m256 t4 = _mm256_unpacklo_ps(Vec8f(v4), Vec8f(v5));
        const __m256 t6 = _mm256_unpacklo_ps(Vec8f(v6), Vec8f(v7));
        const __m256 tt0 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(1, 0, 1, 0));
        const __m256 tt1 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(3, 2, 3, 2));
        const __m256 tt4 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(1, 0, 1, 0));
        const __m256 tt5 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(3, 2, 3, 2));
        x = _mm256_permute2f128_ps(tt0, tt4, 0x20);
        y = _mm256_permute2f128_ps(tt1, tt5, 0x20);

#else // !NFE_USE_AVX

        x = Vec8f{ v0.x, v1.x, v2.x, v3.x, v4.x, v5.x, v6.x, v7.x };
        y = Vec8f{ v0.y, v1.y, v2.y, v3.y, v4.y, v5.y, v6.y, v7.y };

#endif // NFE_USE_AVX
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE const Vec2x8f operator + (const Vec2x8f& rhs) const
    {
        return { x + rhs.x, y + rhs.y };
    }

    NFE_FORCE_INLINE const Vec2x8f operator - (const Vec2x8f& rhs) const
    {
        return { x - rhs.x, y - rhs.y };
    }

    NFE_FORCE_INLINE const Vec2x8f operator * (const Vec2x8f& rhs) const
    {
        return { x * rhs.x, y * rhs.y };
    }

    NFE_FORCE_INLINE const Vec2x8f operator * (const Vec8f& rhs) const
    {
        return { x * rhs, y * rhs };
    }

    NFE_FORCE_INLINE const Vec2x8f operator / (const Vec2x8f& rhs) const
    {
        return { x / rhs.x, y / rhs.y };
    }

    NFE_FORCE_INLINE const Vec2x8f operator * (const float rhs) const
    {
        return { x * rhs, y * rhs };
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE Vec2x8f& operator += (const Vec2x8f& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    NFE_FORCE_INLINE Vec2x8f& operator -= (const Vec2x8f& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    NFE_FORCE_INLINE Vec2x8f& operator *= (const Vec2x8f& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }

    NFE_FORCE_INLINE Vec2x8f& operator /= (const Vec2x8f& rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }

    NFE_FORCE_INLINE Vec2x8f& operator *= (const float rhs)
    {
        x *= rhs;
        y *= rhs;
        return *this;
    }

    NFE_FORCE_INLINE static const Vec2x8f FastReciprocal(const Vec2x8f& v)
    {
        return
        {
            Vec8f::FastReciprocal(v.x),
            Vec8f::FastReciprocal(v.y)
        };
    }

    NFE_FORCE_INLINE static const Vec2x8f MulAndAdd(const Vec2x8f& a, const Vec2x8f& b, const Vec2x8f& c)
    {
        return
        {
            Vec8f::MulAndAdd(a.x, b.x, c.x),
            Vec8f::MulAndAdd(a.y, b.y, c.y)
        };
    }

    NFE_FORCE_INLINE static const Vec2x8f MulAndSub(const Vec2x8f& a, const Vec2x8f& b, const Vec2x8f& c)
    {
        return
        {
            Vec8f::MulAndSub(a.x, b.x, c.x),
            Vec8f::MulAndSub(a.y, b.y, c.y)
        };
    }

    NFE_FORCE_INLINE static const Vec2x8f NegMulAndAdd(const Vec2x8f& a, const Vec2x8f& b, const Vec2x8f& c)
    {
        return
        {
            Vec8f::NegMulAndAdd(a.x, b.x, c.x),
            Vec8f::NegMulAndAdd(a.y, b.y, c.y)
        };
    }

    NFE_FORCE_INLINE static const Vec2x8f NegMulAndSub(const Vec2x8f& a, const Vec2x8f& b, const Vec2x8f& c)
    {
        return
        {
            Vec8f::NegMulAndSub(a.x, b.x, c.x),
            Vec8f::NegMulAndSub(a.y, b.y, c.y)
        };
    }

    //////////////////////////////////////////////////////////////////////////

    // dot product
    NFE_FORCE_INLINE static const Vec8f Dot(const Vec2x8f& a, const Vec2x8f& b)
    {
        // return a.x * b.x + a.y * b.y
        return Vec8f::MulAndAdd(a.x, b.x, a.y * b.y);
    }
    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE static const Vec2x8f Min(const Vec2x8f& a, const Vec2x8f& b)
    {
        return { Vec8f::Min(a.x, b.x), Vec8f::Min(a.y, b.y) };
    }

    NFE_FORCE_INLINE static const Vec2x8f Max(const Vec2x8f& a, const Vec2x8f& b)
    {
        return { Vec8f::Max(a.x, b.x), Vec8f::Max(a.y, b.y) };
    }

};


NFE_FORCE_INLINE const Vec2x8f operator * (const float lhs, const Vec2x8f rhs)
{
    return { lhs * rhs.x, lhs * rhs.y };
}


} // namespace Math
} // namespace NFE
