#pragma once

#include "Math.hpp"
#include "Vec4f.hpp"
#include "Ray.hpp"
#include "Box.hpp"
#include "Triangle.hpp"


namespace NFE {
namespace Math {

enum class IntersectionResult
{
    Outside = 0, //< no intersection
    Inside = 1, //< shapeA is inside shapeB
    Intersect = 2, //< shapeA and shapeB intersects
};

/**
 * Finds the nearest point on a line segment to a @a point.
 * @param[in]  p1,p2 Line segment ends.
 * @param[out] out Nearest point on the segment.
 * @return     Distance to the segment.
 */
NFCOMMON_API float ClosestPointOnSegment(const Vec4f& p, const Vec4f& p1, const Vec4f& p2, Vec4f& out);

/**
 * Template function for intersection tests.
 */
template<typename ShapeTypeA, typename ShapeTypeB>
bool Intersect(const ShapeTypeA& shapeA, const ShapeTypeB& shapeB);

/**
 * Template function for extended intersection tests.
 * @see IntersectionResult
 */
template<typename ShapeTypeA, typename ShapeTypeB>
IntersectionResult IntersectEx(const ShapeTypeA& shapeA, const ShapeTypeB& shapeB);

/**
 * Template function for ray intersection tests.
 * @param dist Distance to the intersection.
 */
template<typename ShapeType>
bool Intersect(const Ray& ray, const ShapeType& shape, Vec4f& dist);

// convert cartesian (x,y,z) to spherical coordinates (phi,theta)
NFCOMMON_API const Vec4f CartesianToSpherical(const Vec4f& input);

// convert spherical (phi,theta) to cartesian coordinates (x,y,z)
NFCOMMON_API const Vec4f SphericalToCartesian(float phi, float theta);

NFE_FORCE_INLINE constexpr float UniformHemispherePdf()
{
    return NFE_MATH_INV_PI / 2.0f;
}

NFE_FORCE_INLINE constexpr float UniformSpherePdf()
{
    return NFE_MATH_INV_PI / 4.0f;
}

NFE_FORCE_INLINE constexpr float UniformSpherePdf(const float radius)
{
    return NFE_MATH_INV_PI / (4.0f * radius * radius);
}

NFE_FORCE_INLINE constexpr float UniformCirclePdf(const float radius)
{
    return 1.0f / (NFE_MATH_PI * Sqr(radius));
}

NFE_FORCE_INLINE constexpr float SphereCapPdf(const float cosTheta)
{
    return 1.0f / (NFE_MATH_2PI * (1.0f - cosTheta));
}

// Given a normalized vector 'n', generate orthonormal vectors 'u' and 'v'
NFCOMMON_API void BuildOrthonormalBasis(const Vec4f& n, Vec4f& u, Vec4f& v);

NFE_FORCE_INLINE float PointLineDistanceSqr(const Vec4f& pointOnLine, const Vec4f& lineDir, const Vec4f& testPoint)
{
    const Vec4f t = testPoint - pointOnLine;
    return Vec4f::Cross3(lineDir, t).SqrLength3() / lineDir.SqrLength3();
}

NFE_FORCE_INLINE float TriangleSurfaceArea(const Vec4f& edge0, const Vec4f& edge1)
{
    const Vec4f cross = Vec4f::Cross3(edge1, edge0);
    return cross.Length3() * 0.5f;
}

NFE_FORCE_INLINE bool Intersect_BoxRay(const Ray& ray, const Box& box, float& outDistance)
{
    // calculate all box planes distances
    Vec4f tmp1 = Vec4f::MulAndSub(box.min, ray.invDir, ray.originDivDir); // box.min * ray.invDir - ray.originDivDir;
    Vec4f tmp2 = Vec4f::MulAndSub(box.max, ray.invDir, ray.originDivDir); // box.max * ray.invDir - ray.originDivDir;
    Vec4f lmin = Vec4f::Min(tmp1, tmp2);
    Vec4f lmax = Vec4f::Max(tmp1, tmp2);

#ifdef NFE_USE_SSE

    // transpose (we need to calculate min and max of X, Y and Z)
    Vec4f lx = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(0, 0, 0, 0));
    Vec4f ly = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(1, 1, 1, 1));
    Vec4f lz = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(2, 2, 2, 2));

    // calculate minimum and maximum plane distances by taking min and max of all 3 components
    lmin = Vec4f::Max(lx, Vec4f::Max(ly, lz));
    lmax = Vec4f::Min(lx, Vec4f::Min(ly, lz));
    outDistance = lmin.x;

    // setup data for final comparison
    lmax = lmax.SplatZ();
    // lmin = [lmin, 0, lmin, 0]
    lmin = _mm_unpacklo_ps(lmin, _mm_setzero_ps());

    // lmax >= lmin && lmax >= 0.0
    // (bits 0, 2)     (bit 1, 3)
    // The check below is a little bit redundant (we perform 4 comparisons), so
    // all 4 comparisons must return success (that's why we check if mask is 0xF).
    return (lmax >= lmin).All();

#else // !NFE_USE_SSE

    // calculate minimum and maximum plane distances by taking min and max of all 3 components
    const float minDist = Max(lmin.x, Max(lmin.y, lmin.z));
    const float maxDist = Min(lmax.x, Min(lmax.y, lmax.z));
    outDistance = minDist;

    return (maxDist >= minDist) && (maxDist >= 0.0f);

#endif // NFE_USE_SSE
}

NFE_FORCE_INLINE bool Intersect_BoxRay_TwoSided(const Ray& ray, const Box& box, float& outNearDist, float& outFarDist)
{
    // calculate all box planes distances
    Vec4f tmp1 = Vec4f::MulAndSub(box.min, ray.invDir, ray.originDivDir); // box.min * ray.invDir - ray.originDivDir;
    Vec4f tmp2 = Vec4f::MulAndSub(box.max, ray.invDir, ray.originDivDir); // box.max * ray.invDir - ray.originDivDir;
    Vec4f lmin = Vec4f::Min(tmp1, tmp2);
    Vec4f lmax = Vec4f::Max(tmp1, tmp2);

#ifdef NFE_USE_SSE

    // transpose (we need to calculate min and max of X, Y and Z)
    Vec4f lx = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(0, 0, 0, 0));
    Vec4f ly = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(1, 1, 1, 1));
    Vec4f lz = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(2, 2, 2, 2));

    // calculate minimum and maximum plane distances by taking min and max of all 3 components
    lmin = Vec4f::Max(lx, Vec4f::Max(ly, lz));
    lmax = Vec4f::Min(lx, Vec4f::Min(ly, lz));

    outNearDist = lmin.x;
    outFarDist  = lmax.z;

#else // !NFE_USE_SSE

    outNearDist = Max(lmin.x, Max(lmin.y, lmin.z));
    outFarDist  = Min(lmax.x, Min(lmax.y, lmax.z));

#endif // NFE_USE_SSE

    return outNearDist < outFarDist;
}

NFE_FORCE_INLINE bool Intersect_TriangleRay(
    const Ray& ray,
    const Vec4f& vertex0, const Vec4f& edge01, const Vec4f& edge02,
    float& outU, float& outV, float& outDistance)
{
    // Based on "Fast, Minimum Storage Ray/Triangle Intersection" by Tomas Möller and Ben Trumbore.

    // calculate distance from vert0 to ray origin
    Vec4f tvec = ray.origin - vertex0;
    Vec4f pvec = Vec4f::Cross3(ray.dir, edge02);
    Vec4f qvec = Vec4f::Cross3(tvec, edge01);

#ifdef NFE_USE_SSE

    Vec4f det = Vec4f::Dot3V(edge01, pvec);
    Vec4f u = Vec4f::Dot3V(tvec, pvec);
    Vec4f v = Vec4f::Dot3V(ray.dir, qvec);
    Vec4f t = Vec4f::Dot3V(edge02, qvec);

    // prepare data to the final comparison
    Vec4f tmp1 = _mm_unpacklo_ps(v, u); // [v, u, v, u]
    Vec4f tmp2 = _mm_unpacklo_ps(u + v, t); // [u+v, t, u+v, t]
    tmp1 = _mm_unpacklo_ps(tmp2, tmp1); // [u+v, v, t, u]
    tmp1 = tmp1 / det; // TODO this is slow, but reciprocal approximation gives bad results (artifacts)
    tmp2 = _mm_set_ss(1.0f); // [1.0, 0.0, 0.0, 0.0]

    outU = tmp1.w;
    outV = tmp1.y;
    outDistance = tmp1.z;

    // At this point, in tmp1 we have: [u, v, t, u + v]
    // and in tmp2 we have:            [0, 0, 0, 1].
    // The intersection occurs if (u > 0 && v > 0 && t > 0 && u + v <= 1),
    // so when performing SSE comparison 3 upper components must return true,
    // and last false, which yields to 0xE bit mask.
    return (tmp1 > tmp2).GetMask() == 0xE;

#else // !NFE_USE_SSE

    // if determinant is near zero, ray lies in plane of triangle
    float det = Vec4f::Dot3(edge01, pvec);

    float u = Vec4f::Dot3(tvec, pvec);
    float v = Vec4f::Dot3(ray.dir, qvec);
    float t = Vec4f::Dot3(edge02, qvec);

    u /= det;
    v /= det;
    t /= det;

    outU = u;
    outV = v;
    outDistance = t;

    return u >= 0.0f && v >= 0.0f && t >= 0.0f && u + v <= 1.0f;

#endif // NFE_USE_SSE
}


} // namespace Math
} // namespace NFE
