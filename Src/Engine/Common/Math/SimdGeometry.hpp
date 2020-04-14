#pragma once

#include "SimdRay.hpp"
#include "SimdBox.hpp"
#include "SimdTriangle.hpp"

// Defining it will replace VBLENDVPS instruction with VMINPS/VMAXPS
// Makes the code faster on Haswell and Broadwell
// Probably not needed on Skylake (and higher) and AMD
#define RT_ARCH_SLOW_BLENDV

namespace NFE {
namespace Math {

template<uint32 NumLanes>
struct SimdTraits
{};

template<>
struct SimdTraits<4>
{
    using Float = Math::Vec4f;
    using FloatMask = Math::VecBool4f;
    using Int32 = Math::Vec4i;
    using Uint32 = Math::Vec4ui;
    using Vec2f = Math::Vec2x4f;
    using Vec3f = Math::Vec3x4f;
};

template<>
struct SimdTraits<8>
{
    using Float = Math::Vec8f;
    using FloatMask = Math::VecBool8f;
    using Int32 = Math::Vec8i;
    using Uint32 = Math::Vec8ui;
    using Vec2f = Math::Vec2x8f;
    using Vec3f = Math::Vec3x8f;
};

template<>
struct SimdTraits<16>
{
    using Float = Math::Vec16f;
    using FloatMask = Math::VecBool16;
    using Int32 = Math::Vec16i;
    using Uint32 = Math::Vec16ui;
    using Vec2f = Math::Vec2x16f;
    using Vec3f = Math::Vec3x16f;
};

template<uint32 NumLanes>
struct Simd
{
    using Float = typename SimdTraits<NumLanes>::Float;
    using FloatMask = typename SimdTraits<NumLanes>::FloatMask;
    using Int32 = typename SimdTraits<NumLanes>::Int32;
    using Uint32 = typename SimdTraits<NumLanes>::Uint32;
    using Vec2f = typename SimdTraits<NumLanes>::Vec2f;
    using Vec3f = typename SimdTraits<NumLanes>::Vec3f;

    using Ray = Math::SimdRay<Vec3f>;
    using Box = Math::SimdBox<Vec3f>;
    using Triangle = Math::SimdTriangle<Vec3f>;

    /*
    template<uint32 Octatnt>
    NFE_FORCE_INLINE const VecBool8f Intersect_BoxRay_Simd8_Octant(
        const Vec3f& rayInvDir,
        const Vec3f& rayOriginDivDir,
        const Box_Simd8& box,
        const Float& maxDistance,
        Float& outDistance)
    {
        static_assert(Octatnt < 8, "Invalid octant");

        const Vec3f tmp1 = Vec3f::MulAndSub(box.min, rayInvDir, rayOriginDivDir);
        const Vec3f tmp2 = Vec3f::MulAndSub(box.max, rayInvDir, rayOriginDivDir);

        constexpr uint32 maskX = Octatnt & 1 ? 1 : 0;
        constexpr uint32 maskY = Octatnt & 2 ? 1 : 0;
        constexpr uint32 maskZ = Octatnt & 4 ? 1 : 0;

        Vec3f lmin, lmax;
        // lmax.x = _mm256_blend_ps(tmp2.x, tmp1.x, maskX);
        // lmax.y = _mm256_blend_ps(tmp2.y, tmp1.y, maskY);
        // lmax.z = _mm256_blend_ps(tmp2.z, tmp1.z, maskZ);
        // lmin.x = _mm256_blend_ps(tmp1.x, tmp2.x, maskX);
        // lmin.y = _mm256_blend_ps(tmp1.y, tmp2.y, maskY);
        // lmin.z = _mm256_blend_ps(tmp1.z, tmp2.z, maskZ);

        lmax.x = Float::Select<maskX,maskX,maskX,maskX>(tmp2.x, tmp1.x);
        lmax.y = Float::Select<maskY,maskY,maskY,maskY>(tmp2.y, tmp1.y);
        lmax.z = Float::Select<maskZ,maskZ,maskZ,maskZ>(tmp2.z, tmp1.z);
        lmin.x = Float::Select<maskX,maskX,maskX,maskX>(tmp1.x, tmp2.x);
        lmin.y = Float::Select<maskY,maskY,maskY,maskY>(tmp1.y, tmp2.y);
        lmin.z = Float::Select<maskZ,maskZ,maskZ,maskZ>(tmp1.z, tmp2.z);

        // calculate minimum and maximum plane distances by taking min and max of all 3 components
        const Float maxT = Float::Min(lmax.z, Float::Min(lmax.x, lmax.y));
        const Float minT = Float::Max(lmin.z, Float::Max(lmin.x, lmin.y));

        outDistance = minT;

    #ifdef NFE_USE_AVX
        const Float cond = _mm256_cmp_ps(Float::Min(maxDistance, maxT), minT, _CMP_GE_OQ);
        return _mm256_andnot_ps(maxT, cond); // trick: replace greater-than-zero compare with and-not
    #else
        const Float zero = Float::Zero();
        return (maxT > zero) & (minT <= maxT) & (maxT <= maxDistance);
    #endif
    }
    */

    NFE_FORCE_INLINE static const FloatMask Intersect_BoxRay(
        const Vec3f& rayInvDir,
        const Vec3f& rayOriginDivDir,
        const Box& box,
        const Float& maxDistance,
        Float& outDistance)
    {
        const Vec3f tmp1 = Vec3f::MulAndSub(box.min, rayInvDir, rayOriginDivDir);
        const Vec3f tmp2 = Vec3f::MulAndSub(box.max, rayInvDir, rayOriginDivDir);

        // TODO we can get rid of this 3 mins and 3 maxes by sorting the rays into octants
        // and processing each octant separately
#if defined(RT_ARCH_SLOW_BLENDV) || !defined(NFE_USE_AVX)
        const Vec3f lmax = Vec3f::Max(tmp1, tmp2);
        const Vec3f lmin = Vec3f::Min(tmp1, tmp2);
#else // RT_ARCH_SLOW_BLENDV
        Vec3f lmin, lmax;
        lmax.x = _mm256_blendv_ps(tmp2.x, tmp1.x, rayInvDir.x);
        lmax.y = _mm256_blendv_ps(tmp2.y, tmp1.y, rayInvDir.y);
        lmax.z = _mm256_blendv_ps(tmp2.z, tmp1.z, rayInvDir.z);
        lmin.x = _mm256_blendv_ps(tmp1.x, tmp2.x, rayInvDir.x);
        lmin.y = _mm256_blendv_ps(tmp1.y, tmp2.y, rayInvDir.y);
        lmin.z = _mm256_blendv_ps(tmp1.z, tmp2.z, rayInvDir.z);
#endif // RT_ARCH_SLOW_BLENDV

        // calculate minimum and maximum plane distances by taking min and max of all 3 components
        const Float maxT = Float::Min(lmax.z, Float::Min(lmax.x, lmax.y));
        const Float minT = Float::Max(lmin.z, Float::Max(lmin.x, lmin.y));

        outDistance = minT;

/*
#ifdef NFE_USE_AVX
        // return (maxT > 0 && minT <= maxT && maxT <= maxDistance)
        const Float cond = _mm256_cmp_ps(Float::Min(maxDistance, maxT), minT, _CMP_GE_OQ);
        return _mm256_andnot_ps(maxT, cond); // trick: replace greater-than-zero compare with and-not
#else
*/
        return (maxT > Float::Zero()) & (Float::Min(maxDistance, maxT) >= minT);
//#endif
    }

    NFE_FORCE_INLINE static const FloatMask Intersect_BoxRay_TwoSided(
        const Vec3f& rayInvDir,
        const Vec3f& rayOriginDivDir,
        const Box& box,
        const Float& maxDistance,
        Float& outNearDist,
        Float& outFarDist)
    {
        const Vec3f tmp1 = Vec3f::MulAndSub(box.min, rayInvDir, rayOriginDivDir);
        const Vec3f tmp2 = Vec3f::MulAndSub(box.max, rayInvDir, rayOriginDivDir);

#if defined(RT_ARCH_SLOW_BLENDV) || !defined(NFE_USE_AVX)
        const Vec3f lmax = Vec3f::Max(tmp1, tmp2);
        const Vec3f lmin = Vec3f::Min(tmp1, tmp2);
#else // RT_ARCH_SLOW_BLENDV
        Vec3f lmin, lmax;
        lmax.x = _mm256_blendv_ps(tmp2.x, tmp1.x, rayInvDir.x);
        lmax.y = _mm256_blendv_ps(tmp2.y, tmp1.y, rayInvDir.y);
        lmax.z = _mm256_blendv_ps(tmp2.z, tmp1.z, rayInvDir.z);
        lmin.x = _mm256_blendv_ps(tmp1.x, tmp2.x, rayInvDir.x);
        lmin.y = _mm256_blendv_ps(tmp1.y, tmp2.y, rayInvDir.y);
        lmin.z = _mm256_blendv_ps(tmp1.z, tmp2.z, rayInvDir.z);
#endif // RT_ARCH_SLOW_BLENDV

        // calculate minimum and maximum plane distances by taking min and max of all 3 components
        const Float maxT = Float::Min(lmax.z, Float::Min(lmax.x, lmax.y));
        const Float minT = Float::Max(lmin.z, Float::Max(lmin.x, lmin.y));

        outNearDist = minT;
        outFarDist = maxT;

#ifdef NFE_USE_AVX
        // return (maxT > 0 && minT <= maxT && maxT <= maxDistance)
        const Float cond = _mm256_cmp_ps(Float::Min(maxDistance, maxT), minT, _CMP_GE_OQ);
        return VecBool8f(_mm256_andnot_ps(maxT, cond)); // trick: replace greater-than-zero compare with and-not
#else
        const Float zero = Float::Zero();
        return (maxT > zero) & (minT <= maxT) & (maxT <= maxDistance);
#endif
    }

    NFE_INLINE static const FloatMask Intersect_TriangleRay(
        const Vec3f& rayDir,
        const Vec3f& rayOrigin,
        const Triangle& tri,
        const Float& maxDistance,
        Float& outU,
        Float& outV,
        Float& outDist)
    {
        // Muller-Trumbore algorithm

        const Float one(1.0f);

        // begin calculating determinant - also used to calculate U parameter
        const Vec3f pvec = Vec3f::Cross(rayDir, tri.edge2);

        // if determinant is near zero, ray lies in plane of triangle
        const Float det = Vec3f::Dot(tri.edge1, pvec);
        const Float invDet = one / det;

        // calculate distance from vert0 to ray origin
        const Vec3f tvec = rayOrigin - tri.v0;

        // prepare to test V parameter
        const Vec3f qvec = Vec3f::Cross(tvec, tri.edge1);

        const Float u = invDet * Vec3f::Dot(tvec, pvec);
        const Float v = invDet * Vec3f::Dot(rayDir, qvec);
        const Float t = invDet * Vec3f::Dot(tri.edge2, qvec);

        outU = u;
        outV = v;
        outDist = t;

        /*
#ifdef NFE_USE_AVX2
        // t > 0 && t < maxDist
        const __m256i condA = _mm256_andnot_si256(_mm256_castps_si256(u), _mm256_cmpgt_epi32(_mm256_castps_si256(maxDistance), _mm256_castps_si256(t)));
        // u > 0 && u + v < 1
        const __m256i condB = _mm256_andnot_si256(_mm256_castps_si256(t), _mm256_castps_si256(_mm256_cmp_ps(u + v, one, _CMP_LE_OQ)));
        // v > 0
        return VecBool8f(_mm256_andnot_si256(v, _mm256_and_si256(condA, condB)));
#elif defined(NFE_USE_AVX)
        // u > 0 && v > 0 && t > 0 && u + v < 1 && t < maxDist
        const Float condA = _mm256_andnot_ps(u, _mm256_cmp_ps(t, maxDistance, _CMP_LT_OQ));
        const Float condB = _mm256_andnot_ps(t, _mm256_cmp_ps(u + v, one, _CMP_LE_OQ));
        return _mm256_andnot_ps(v, _mm256_and_ps(condA, condB));
#else
*/
        const Float zero = Float::Zero();
        return (u > zero) & (v > zero) & (t > zero) & (u + v < one) & (t < maxDistance);
//#endif
    }

};


} // namespace Math
} // namespace NFE
