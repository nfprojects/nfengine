#pragma once

#include "Math.hpp"
#include "Vec8f.hpp"

namespace NFE {
namespace Math {

/**
 * Eight 3D vectors (SIMD version, AVX accelerated).
 */
class NFE_ALIGN(32) Vec3x8f
{
public:
    Vec8f x;
    Vec8f y;
    Vec8f z;

    NFE_FORCE_INLINE Vec3x8f() = default;
    NFE_FORCE_INLINE Vec3x8f(const Vec3x8f&) = default;
    NFE_FORCE_INLINE Vec3x8f& operator = (const Vec3x8f&) = default;

    NFE_FORCE_INLINE static const Vec3x8f Zero()
    {
        return { Vec8f::Zero(), Vec8f::Zero(), Vec8f::Zero() };
    }

    NFE_FORCE_INLINE static const Vec3x8f One()
    {
        return { Vec8f(1.0f), Vec8f(1.0f), Vec8f(1.0f) };
    }

    NFE_FORCE_INLINE Vec3x8f(const Vec8f& x, const Vec8f& y, const Vec8f& z)
        : x(x), y(y), z(z)
    {}

    // splat value to all the components
    NFE_FORCE_INLINE explicit Vec3x8f(const Vec8f& s)
        : x(s), y(s), z(s)
    {}

    // splat single 3D vector
    NFE_FORCE_INLINE explicit Vec3x8f(const Vec4f& v)
    {
#ifdef NFE_USE_AVX
        const Vec8f temp{ v, v }; // copy "v" onto both AVX lanes
        x = _mm256_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 0, 0, 0));
        y = _mm256_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 1, 1, 1));
        z = _mm256_shuffle_ps(temp, temp, _MM_SHUFFLE(2, 2, 2, 2));
#else
        x = Vec8f{ v.x };
        y = Vec8f{ v.y };
        z = Vec8f{ v.z };
#endif // NFE_USE_AVX
    }

    // splat single scalar to all components an elements
    NFE_FORCE_INLINE explicit Vec3x8f(const float f)
        : x(f) , y(f) , z(f)
    {}

    // splat single 3D vector
    NFE_FORCE_INLINE explicit Vec3x8f(const Vec3f& v)
        : x(v.x), y(v.y), z(v.z)
    {}

    // build from eight 3D vectors
    NFE_FORCE_INLINE Vec3x8f(const Vec4f& v0, const Vec4f& v1, const Vec4f& v2, const Vec4f& v3,
                              const Vec4f& v4, const Vec4f& v5, const Vec4f& v6, const Vec4f& v7)
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
        // z = [ v0.z, v1.z, v2.z, v3.z, v4.z, v5.z, v6.z, v7.z ]
        //
        // note that "w" component is dropped

#ifdef NFE_USE_AVX

        const __m256 t0 = _mm256_unpacklo_ps(Vec8f(v0), Vec8f(v1));
        const __m256 t1 = _mm256_unpackhi_ps(Vec8f(v0), Vec8f(v1));
        const __m256 t2 = _mm256_unpacklo_ps(Vec8f(v2), Vec8f(v3));
        const __m256 t3 = _mm256_unpackhi_ps(Vec8f(v2), Vec8f(v3));
        const __m256 t4 = _mm256_unpacklo_ps(Vec8f(v4), Vec8f(v5));
        const __m256 t5 = _mm256_unpackhi_ps(Vec8f(v4), Vec8f(v5));
        const __m256 t6 = _mm256_unpacklo_ps(Vec8f(v6), Vec8f(v7));
        const __m256 t7 = _mm256_unpackhi_ps(Vec8f(v6), Vec8f(v7));
        const __m256 tt0 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(1, 0, 1, 0));
        const __m256 tt1 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(3, 2, 3, 2));
        const __m256 tt2 = _mm256_shuffle_ps(t1, t3, _MM_SHUFFLE(1, 0, 1, 0));
        const __m256 tt4 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(1, 0, 1, 0));
        const __m256 tt5 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(3, 2, 3, 2));
        const __m256 tt6 = _mm256_shuffle_ps(t5, t7, _MM_SHUFFLE(1, 0, 1, 0));
        x = _mm256_permute2f128_ps(tt0, tt4, 0x20);
        y = _mm256_permute2f128_ps(tt1, tt5, 0x20);
        z = _mm256_permute2f128_ps(tt2, tt6, 0x20);

#else // !NFE_USE_AVX

        x = Vec8f{ v0.x, v1.x, v2.x, v3.x, v4.x, v5.x, v6.x, v7.x };
        y = Vec8f{ v0.y, v1.y, v2.y, v3.y, v4.y, v5.y, v6.y, v7.y };
        z = Vec8f{ v0.z, v1.z, v2.z, v3.z, v4.z, v5.z, v6.z, v7.z };

#endif // NFE_USE_AVX
    }

    // unpack to 8x Vec4f
    NFE_FORCE_INLINE void Unpack(Vec4f output[8]) const
    {
#ifdef NFE_USE_AVX

        __m256 row0 = x;
        __m256 row1 = y;
        __m256 row2 = z;
        __m256 row3 = _mm256_setzero_ps();
        __m256 tmp3, tmp2, tmp1, tmp0;

        tmp0 = _mm256_shuffle_ps(row0, row1, 0x44);
        tmp2 = _mm256_shuffle_ps(row0, row1, 0xEE);
        tmp1 = _mm256_shuffle_ps(row2, row3, 0x44);
        tmp3 = _mm256_shuffle_ps(row2, row3, 0xEE);
        row0 = _mm256_shuffle_ps(tmp0, tmp1, 0x88);
        row1 = _mm256_shuffle_ps(tmp0, tmp1, 0xDD);
        row2 = _mm256_shuffle_ps(tmp2, tmp3, 0x88);
        row3 = _mm256_shuffle_ps(tmp2, tmp3, 0xDD);

        output[0] = _mm256_castps256_ps128(row0);
        output[1] = _mm256_castps256_ps128(row1);
        output[2] = _mm256_castps256_ps128(row2);
        output[3] = _mm256_castps256_ps128(row3);
        output[4] = _mm256_extractf128_ps(row0, 1);
        output[5] = _mm256_extractf128_ps(row1, 1);
        output[6] = _mm256_extractf128_ps(row2, 1);
        output[7] = _mm256_extractf128_ps(row3, 1);

#else // !NFE_USE_AVX

        for (uint32 i = 0; i < 8; ++i)
        {
            output[i] = Vec4f(x[i], y[i], z[i]);
        }

#endif // NFE_USE_AVX
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE const Vec3x8f operator - () const
    {
        return { -x, -y, -z };
    }

    NFE_FORCE_INLINE const Vec3x8f operator + (const Vec3x8f& rhs) const
    {
        return { x + rhs.x, y + rhs.y, z + rhs.z };
    }

    NFE_FORCE_INLINE const Vec3x8f operator - (const Vec3x8f& rhs) const
    {
        return { x - rhs.x, y - rhs.y, z - rhs.z };
    }

    NFE_FORCE_INLINE const Vec3x8f operator * (const Vec3x8f& rhs) const
    {
        return { x * rhs.x, y * rhs.y,z * rhs.z };
    }

    NFE_FORCE_INLINE const Vec3x8f operator * (const Vec8f& rhs) const
    {
        return { x * rhs, y * rhs, z * rhs };
    }

    NFE_FORCE_INLINE const Vec3x8f operator / (const Vec3x8f& rhs) const
    {
        return { x / rhs.x, y / rhs.y, z / rhs.z };
    }

    NFE_FORCE_INLINE const Vec3x8f operator * (const float rhs) const
    {
        return { x * rhs, y * rhs, z * rhs };
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE Vec3x8f& operator += (const Vec3x8f& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    NFE_FORCE_INLINE Vec3x8f& operator -= (const Vec3x8f& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    NFE_FORCE_INLINE Vec3x8f& operator *= (const Vec3x8f& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }

    NFE_FORCE_INLINE Vec3x8f& operator /= (const Vec3x8f& rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        return *this;
    }

    NFE_FORCE_INLINE Vec3x8f& operator *= (const Vec8f& rhs)
    {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }

    NFE_FORCE_INLINE Vec3x8f& operator *= (const float rhs)
    {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }

    NFE_FORCE_INLINE static const Vec3x8f Reciprocal(const Vec3x8f& v)
    {
        return
        {
            Vec8f::Reciprocal(v.x),
            Vec8f::Reciprocal(v.y),
            Vec8f::Reciprocal(v.z)
        };
    }

    NFE_FORCE_INLINE static const Vec3x8f FastReciprocal(const Vec3x8f& v)
    {
        return
        {
            Vec8f::FastReciprocal(v.x),
            Vec8f::FastReciprocal(v.y),
            Vec8f::FastReciprocal(v.z)
        };
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE static const Vec3x8f MulAndAdd(const Vec3x8f& a, const Vec3x8f& b, const Vec3x8f& c)
    {
        return { Vec8f::MulAndAdd(a.x, b.x, c.x), Vec8f::MulAndAdd(a.y, b.y, c.y), Vec8f::MulAndAdd(a.z, b.z, c.z) };
    }

    NFE_FORCE_INLINE static const Vec3x8f MulAndSub(const Vec3x8f& a, const Vec3x8f& b, const Vec3x8f& c)
    {
        return { Vec8f::MulAndSub(a.x, b.x, c.x), Vec8f::MulAndSub(a.y, b.y, c.y), Vec8f::MulAndSub(a.z, b.z, c.z) };
    }

    NFE_FORCE_INLINE static const Vec3x8f NegMulAndAdd(const Vec3x8f& a, const Vec3x8f& b, const Vec3x8f& c)
    {
        return { Vec8f::NegMulAndAdd(a.x, b.x, c.x), Vec8f::NegMulAndAdd(a.y, b.y, c.y), Vec8f::NegMulAndAdd(a.z, b.z, c.z) };
    }

    NFE_FORCE_INLINE static const Vec3x8f NegMulAndSub(const Vec3x8f& a, const Vec3x8f& b, const Vec3x8f& c)
    {
        return { Vec8f::NegMulAndSub(a.x, b.x, c.x), Vec8f::NegMulAndSub(a.y, b.y, c.y), Vec8f::NegMulAndSub(a.z, b.z, c.z) };
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE static const Vec3x8f MulAndAdd(const Vec3x8f& a, const Vec8f& b, const Vec3x8f& c)
    {
        return { Vec8f::MulAndAdd(a.x, b, c.x), Vec8f::MulAndAdd(a.y, b, c.y), Vec8f::MulAndAdd(a.z, b, c.z) };
    }

    NFE_FORCE_INLINE static const Vec3x8f MulAndSub(const Vec3x8f& a, const Vec8f& b, const Vec3x8f& c)
    {
        return { Vec8f::MulAndSub(a.x, b, c.x), Vec8f::MulAndSub(a.y, b, c.y), Vec8f::MulAndSub(a.z, b, c.z) };
    }

    NFE_FORCE_INLINE static const Vec3x8f NegMulAndAdd(const Vec3x8f& a, const Vec8f& b, const Vec3x8f& c)
    {
        return { Vec8f::NegMulAndAdd(a.x, b, c.x), Vec8f::NegMulAndAdd(a.y, b, c.y), Vec8f::NegMulAndAdd(a.z, b, c.z) };
    }

    NFE_FORCE_INLINE static const Vec3x8f NegMulAndSub(const Vec3x8f& a, const Vec8f& b, const Vec3x8f& c)
    {
        return { Vec8f::NegMulAndSub(a.x, b, c.x), Vec8f::NegMulAndSub(a.y, b, c.y), Vec8f::NegMulAndSub(a.z, b, c.z) };
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE static const Vec3x8f MulAndAdd(const Vec3x8f& a, const Vec3x8f& b, const Vec8f& c)
    {
        return { Vec8f::MulAndAdd(a.x, b.x, c), Vec8f::MulAndAdd(a.y, b.y, c), Vec8f::MulAndAdd(a.z, b.z, c) };
    }

    NFE_FORCE_INLINE static const Vec3x8f MulAndSub(const Vec3x8f& a, const Vec3x8f& b, const Vec8f& c)
    {
        return { Vec8f::MulAndSub(a.x, b.x, c), Vec8f::MulAndSub(a.y, b.y, c), Vec8f::MulAndSub(a.z, b.z, c) };
    }

    NFE_FORCE_INLINE static const Vec3x8f NegMulAndAdd(const Vec3x8f& a, const Vec3x8f& b, const Vec8f& c)
    {
        return { Vec8f::NegMulAndAdd(a.x, b.x, c), Vec8f::NegMulAndAdd(a.y, b.y, c), Vec8f::NegMulAndAdd(a.z, b.z, c) };
    }

    NFE_FORCE_INLINE static const Vec3x8f NegMulAndSub(const Vec3x8f& a, const Vec3x8f& b, const Vec8f& c)
    {
        return { Vec8f::NegMulAndSub(a.x, b.x, c), Vec8f::NegMulAndSub(a.y, b.y, c), Vec8f::NegMulAndSub(a.z, b.z, c) };
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE static const Vec3x8f MulAndAdd(const Vec3x8f& a, const Vec8f& b, const Vec8f& c)
    {
        return { Vec8f::MulAndAdd(a.x, b, c), Vec8f::MulAndAdd(a.y, b, c), Vec8f::MulAndAdd(a.z, b, c) };
    }

    NFE_FORCE_INLINE static const Vec3x8f MulAndSub(const Vec3x8f& a, const Vec8f& b, const Vec8f& c)
    {
        return { Vec8f::MulAndSub(a.x, b, c), Vec8f::MulAndSub(a.y, b, c), Vec8f::MulAndSub(a.z, b, c) };
    }

    NFE_FORCE_INLINE static const Vec3x8f NegMulAndAdd(const Vec3x8f& a, const Vec8f& b, const Vec8f& c)
    {
        return { Vec8f::NegMulAndAdd(a.x, b, c), Vec8f::NegMulAndAdd(a.y, b, c), Vec8f::NegMulAndAdd(a.z, b, c) };
    }

    NFE_FORCE_INLINE static const Vec3x8f NegMulAndSub(const Vec3x8f& a, const Vec8f& b, const Vec8f& c)
    {
        return { Vec8f::NegMulAndSub(a.x, b, c), Vec8f::NegMulAndSub(a.y, b, c), Vec8f::NegMulAndSub(a.z, b, c) };
    }

    //////////////////////////////////////////////////////////////////////////

    // 3D dot product
    NFE_FORCE_INLINE static const Vec8f Dot(const Vec3x8f& a, const Vec3x8f& b)
    {
        // return a.x * b.x + a.y * b.y + a.z * b.z;
        return Vec8f::MulAndAdd(a.x, b.x, Vec8f::MulAndAdd(a.y, b.y, a.z * b.z));
    }

    // 3D cross product
    NFE_FORCE_INLINE static const Vec3x8f Cross(const Vec3x8f& a, const Vec3x8f& b)
    {
        return {
            Vec8f::NegMulAndAdd(a.z, b.y, a.y * b.z),
            Vec8f::NegMulAndAdd(a.x, b.z, a.z * b.x),
            Vec8f::NegMulAndAdd(a.y, b.x, a.x * b.y)
        };
    }

    NFE_FORCE_INLINE const Vec8f SqrLength() const
    {
        return Dot(*this, *this);
    }

    NFE_FORCE_INLINE const Vec3x8f Normalized() const
    {
        const Vec8f invLength = Vec8f::Reciprocal(Vec8f::Sqrt(SqrLength()));
        return (*this) * invLength;
    }

    //////////////////////////////////////////////////////////////////////////

    NFE_FORCE_INLINE static const Vec3x8f Min(const Vec3x8f& a, const Vec3x8f& b)
    {
        return { Vec8f::Min(a.x, b.x), Vec8f::Min(a.y, b.y), Vec8f::Min(a.z, b.z) };
    }

    NFE_FORCE_INLINE static const Vec3x8f Max(const Vec3x8f& a, const Vec3x8f& b)
    {
        return { Vec8f::Max(a.x, b.x), Vec8f::Max(a.y, b.y), Vec8f::Max(a.z, b.z) };
    }

};


} // namespace Math
} // namespace NFE
