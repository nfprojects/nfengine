#pragma once

#include "Simd8Ray.hpp"
#include "Simd8Box.hpp"
#include "Simd8Triangle.hpp"

// Defining it will replace VBLENDVPS instruction with VMINPS/VMAXPS
// Makes the code faster on Haswell and Broadwell
// Probably not needed on Skylake (and higher) and AMD
#define RT_ARCH_SLOW_BLENDV

namespace NFE {
namespace Math {

template<uint32 Octatnt>
NFE_FORCE_INLINE const VecBool8f Intersect_BoxRay_Simd8_Octant(
    const Vec3x8f& rayInvDir,
    const Vec3x8f& rayOriginDivDir,
    const Box_Simd8& box,
    const Vec8f& maxDistance,
    Vec8f& outDistance)
{
    static_assert(Octatnt < 8, "Invalid octant");

    const Vec3x8f tmp1 = Vec3x8f::MulAndSub(box.min, rayInvDir, rayOriginDivDir);
    const Vec3x8f tmp2 = Vec3x8f::MulAndSub(box.max, rayInvDir, rayOriginDivDir);

    constexpr uint32 maskX = Octatnt & 1 ? 1 : 0;
    constexpr uint32 maskY = Octatnt & 2 ? 1 : 0;
    constexpr uint32 maskZ = Octatnt & 4 ? 1 : 0;

    Vec3x8f lmin, lmax;
    // lmax.x = _mm256_blend_ps(tmp2.x, tmp1.x, maskX);
    // lmax.y = _mm256_blend_ps(tmp2.y, tmp1.y, maskY);
    // lmax.z = _mm256_blend_ps(tmp2.z, tmp1.z, maskZ);
    // lmin.x = _mm256_blend_ps(tmp1.x, tmp2.x, maskX);
    // lmin.y = _mm256_blend_ps(tmp1.y, tmp2.y, maskY);
    // lmin.z = _mm256_blend_ps(tmp1.z, tmp2.z, maskZ);

    lmax.x = Vec8f::Select<maskX,maskX,maskX,maskX>(tmp2.x, tmp1.x);
    lmax.y = Vec8f::Select<maskY,maskY,maskY,maskY>(tmp2.y, tmp1.y);
    lmax.z = Vec8f::Select<maskZ,maskZ,maskZ,maskZ>(tmp2.z, tmp1.z);
    lmin.x = Vec8f::Select<maskX,maskX,maskX,maskX>(tmp1.x, tmp2.x);
    lmin.y = Vec8f::Select<maskY,maskY,maskY,maskY>(tmp1.y, tmp2.y);
    lmin.z = Vec8f::Select<maskZ,maskZ,maskZ,maskZ>(tmp1.z, tmp2.z);

    // calculate minimum and maximum plane distances by taking min and max of all 3 components
    const Vec8f maxT = Vec8f::Min(lmax.z, Vec8f::Min(lmax.x, lmax.y));
    const Vec8f minT = Vec8f::Max(lmin.z, Vec8f::Max(lmin.x, lmin.y));

    outDistance = minT;

#ifdef NFE_USE_AVX
    const Vec8f cond = _mm256_cmp_ps(Vec8f::Min(maxDistance, maxT), minT, _CMP_GE_OQ);
    return _mm256_andnot_ps(maxT, cond); // trick: replace greater-than-zero compare with and-not
#else
    const Vec8f zero = Vec8f::Zero();
    return (maxT > zero) & (minT <= maxT) & (maxT <= maxDistance);
#endif
}

NFE_FORCE_INLINE const VecBool8f Intersect_BoxRay_Simd8(
    const Vec3x8f& rayInvDir,
    const Vec3x8f& rayOriginDivDir,
    const Box_Simd8& box,
    const Vec8f& maxDistance,
    Vec8f& outDistance)
{
    const Vec3x8f tmp1 = Vec3x8f::MulAndSub(box.min, rayInvDir, rayOriginDivDir);
    const Vec3x8f tmp2 = Vec3x8f::MulAndSub(box.max, rayInvDir, rayOriginDivDir);

    // TODO we can get rid of this 3 mins and 3 maxes by sorting the rays into octants
    // and processing each octant separately
#if defined(RT_ARCH_SLOW_BLENDV) || !defined(NFE_USE_AVX)
    const Vec3x8f lmax = Vec3x8f::Max(tmp1, tmp2);
    const Vec3x8f lmin = Vec3x8f::Min(tmp1, tmp2);
#else // RT_ARCH_SLOW_BLENDV
    Vec3x8f lmin, lmax;
    lmax.x = _mm256_blendv_ps(tmp2.x, tmp1.x, rayInvDir.x);
    lmax.y = _mm256_blendv_ps(tmp2.y, tmp1.y, rayInvDir.y);
    lmax.z = _mm256_blendv_ps(tmp2.z, tmp1.z, rayInvDir.z);
    lmin.x = _mm256_blendv_ps(tmp1.x, tmp2.x, rayInvDir.x);
    lmin.y = _mm256_blendv_ps(tmp1.y, tmp2.y, rayInvDir.y);
    lmin.z = _mm256_blendv_ps(tmp1.z, tmp2.z, rayInvDir.z);
#endif // RT_ARCH_SLOW_BLENDV

    // calculate minimum and maximum plane distances by taking min and max of all 3 components
    const Vec8f maxT = Vec8f::Min(lmax.z, Vec8f::Min(lmax.x, lmax.y));
    const Vec8f minT = Vec8f::Max(lmin.z, Vec8f::Max(lmin.x, lmin.y));

    outDistance = minT;

#ifdef NFE_USE_AVX
    // return (maxT > 0 && minT <= maxT && maxT <= maxDistance)
    const Vec8f cond = _mm256_cmp_ps(Vec8f::Min(maxDistance, maxT), minT, _CMP_GE_OQ);
    return _mm256_andnot_ps(maxT, cond); // trick: replace greater-than-zero compare with and-not
#else
    const Vec8f zero = Vec8f::Zero();
    return (maxT > zero) & (minT <= maxT) & (maxT <= maxDistance);
#endif
}

NFE_FORCE_INLINE const VecBool8f Intersect_BoxRay_TwoSided_Simd8(
    const Vec3x8f& rayInvDir,
    const Vec3x8f& rayOriginDivDir,
    const Box_Simd8& box,
    const Vec8f& maxDistance,
    Vec8f& outNearDist,
    Vec8f& outFarDist)
{
    const Vec3x8f tmp1 = Vec3x8f::MulAndSub(box.min, rayInvDir, rayOriginDivDir);
    const Vec3x8f tmp2 = Vec3x8f::MulAndSub(box.max, rayInvDir, rayOriginDivDir);

#if defined(RT_ARCH_SLOW_BLENDV) || !defined(NFE_USE_AVX)
    const Vec3x8f lmax = Vec3x8f::Max(tmp1, tmp2);
    const Vec3x8f lmin = Vec3x8f::Min(tmp1, tmp2);
#else // RT_ARCH_SLOW_BLENDV
    Vec3x8f lmin, lmax;
    lmax.x = _mm256_blendv_ps(tmp2.x, tmp1.x, rayInvDir.x);
    lmax.y = _mm256_blendv_ps(tmp2.y, tmp1.y, rayInvDir.y);
    lmax.z = _mm256_blendv_ps(tmp2.z, tmp1.z, rayInvDir.z);
    lmin.x = _mm256_blendv_ps(tmp1.x, tmp2.x, rayInvDir.x);
    lmin.y = _mm256_blendv_ps(tmp1.y, tmp2.y, rayInvDir.y);
    lmin.z = _mm256_blendv_ps(tmp1.z, tmp2.z, rayInvDir.z);
#endif // RT_ARCH_SLOW_BLENDV

    // calculate minimum and maximum plane distances by taking min and max of all 3 components
    const Vec8f maxT = Vec8f::Min(lmax.z, Vec8f::Min(lmax.x, lmax.y));
    const Vec8f minT = Vec8f::Max(lmin.z, Vec8f::Max(lmin.x, lmin.y));

    outNearDist = minT;
    outFarDist = maxT;

#ifdef NFE_USE_AVX
    // return (maxT > 0 && minT <= maxT && maxT <= maxDistance)
    const Vec8f cond = _mm256_cmp_ps(Vec8f::Min(maxDistance, maxT), minT, _CMP_GE_OQ);
    return VecBool8f(_mm256_andnot_ps(maxT, cond)); // trick: replace greater-than-zero compare with and-not
#else
    const Vec8f zero = Vec8f::Zero();
    return (maxT > zero) & (minT <= maxT) & (maxT <= maxDistance);
#endif
}

NFE_INLINE const VecBool8f Intersect_TriangleRay_Simd8(
    const Vec3x8f& rayDir,
    const Vec3x8f& rayOrigin,
    const Triangle_Simd8& tri,
    const Vec8f& maxDistance,
    Vec8f& outU,
    Vec8f& outV,
    Vec8f& outDist)
{
    // Muller-Trumbore algorithm

    const Vec8f one = VECTOR8_ONE;

    // begin calculating determinant - also used to calculate U parameter
    const Vec3x8f pvec = Vec3x8f::Cross(rayDir, tri.edge2);

    // if determinant is near zero, ray lies in plane of triangle
    const Vec8f det = Vec3x8f::Dot(tri.edge1, pvec);
    const Vec8f invDet = one / det;

    // calculate distance from vert0 to ray origin
    const Vec3x8f tvec = rayOrigin - tri.v0;

    // prepare to test V parameter
    const Vec3x8f qvec = Vec3x8f::Cross(tvec, tri.edge1);

    const Vec8f u = invDet * Vec3x8f::Dot(tvec, pvec);
    const Vec8f v = invDet * Vec3x8f::Dot(rayDir, qvec);
    const Vec8f t = invDet * Vec3x8f::Dot(tri.edge2, qvec);

    outU = u;
    outV = v;
    outDist = t;

#ifdef NFE_USE_AVX2
    // t > 0 && t < maxDist
    const __m256i condA = _mm256_andnot_si256(_mm256_castps_si256(u), _mm256_cmpgt_epi32(_mm256_castps_si256(maxDistance), _mm256_castps_si256(t)));
    // u > 0 && u + v < 1
    const __m256i condB = _mm256_andnot_si256(_mm256_castps_si256(t), _mm256_castps_si256(_mm256_cmp_ps(u + v, one, _CMP_LE_OQ)));
    // v > 0
    return VecBool8f(_mm256_andnot_si256(v, _mm256_and_si256(condA, condB)));
#elif defined(NFE_USE_AVX)
    // u > 0 && v > 0 && t > 0 && u + v < 1 && t < maxDist
    const Vec8f condA = _mm256_andnot_ps(u, _mm256_cmp_ps(t, maxDistance, _CMP_LT_OQ));
    const Vec8f condB = _mm256_andnot_ps(t, _mm256_cmp_ps(u + v, one, _CMP_LE_OQ));
    return _mm256_andnot_ps(v, _mm256_and_ps(condA, condB));
#else
    const Vec8f zero = Vec8f::Zero();
    return (u > zero) & (v > zero) & (t > zero) & (u + v < one) & (t < maxDistance);
#endif
}


} // namespace Math
} // namespace NFE
