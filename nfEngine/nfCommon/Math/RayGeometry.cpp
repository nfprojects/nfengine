/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Ray intersection functions.
 */

#include "../PCH.hpp"
#include "Geometry.hpp"
#include "Box.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"
#include "Ray.hpp"

namespace NFE {
namespace Math {

// Ray-Box intersection functions =================================================================

NFE_INLINE bool RayBoxIntersectInline(const Ray& ray, const Box& box, Vector& dist)
{
    // TODO: non-SSE version

    Vector lmin, lmax, tmp1, tmp2;
    tmp1 = (box.min - ray.origin) * ray.invDir;
    tmp2 = (box.max - ray.origin) * ray.invDir;
    lmin = VectorMin(tmp1, tmp2);
    lmax = VectorMax(tmp1, tmp2);

#ifdef NFE_USE_SSE
    Vector lx = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(0, 0, 0, 0));
    Vector ly = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(1, 1, 1, 1));
    Vector lz = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(2, 2, 2, 2));

    lmin = _mm_max_ps(lx, _mm_max_ps(ly, lz));
    dist = lmin.SplatX();
    lmax = _mm_min_ps(lx, _mm_min_ps(ly, lz));
    lmax = _mm_shuffle_ps(lmax, lmax, _MM_SHUFFLE(2, 2, 2, 2));
    lmin = _mm_shuffle_ps(lmin, _mm_setzero_ps(), _MM_SHUFFLE(0, 0, 0, 0));

    return _mm_movemask_ps(_mm_cmpge_ps(lmax, lmin)) == 15;
#else
    float minDist = Max(lmin[0], Max(lmin[1], lmin[2]));
    float maxDist = Min(lmax[0], Min(lmax[1], lmax[2]));
    dist = VectorSplat(minDist);
    return maxDist >= minDist && maxDist >= 0.0f;
#endif // NFE_USE_SSE
}

template<> NFCOMMON_API
bool Intersect(const Ray& ray, const Box& box)
{
    Vector unusedDist;
    return RayBoxIntersectInline(ray, box, unusedDist);
}

template<> NFCOMMON_API
bool Intersect(const Ray& ray, const Box& box, Vector& dist)
{
    return RayBoxIntersectInline(ray, box, dist);
}


// Ray-Triangle intersection functions ============================================================

NFE_INLINE bool RayTriangleIntersectInline(const Ray& ray, const Triangle& tri, Vector& dist)
{
    // TODO: non-SSE version

    Vector edge0 = tri.v1 - tri.v0;
    Vector edge1 = tri.v2 - tri.v0;
    Vector pvec = VectorCross3(ray.dir, edge1);
    Vector tvec = ray.origin - tri.v0;
    Vector qvec = VectorCross3(tvec, edge0);

#ifdef NFE_USE_SSE
    Vector det = VectorDot3(edge0, pvec);
    Vector u = VectorDot3(tvec, pvec);
    Vector v = VectorDot3(ray.dir, qvec);
    Vector t = VectorDot3(edge1, qvec);
    Vector uv_sum = _mm_add_ps(u, v);
    Vector tmp1 = _mm_shuffle_ps(v, u, _MM_SHUFFLE(0, 0, 0, 0));
    Vector tmp2 = _mm_shuffle_ps(uv_sum, t, _MM_SHUFFLE(0, 0, 0, 0));
    tmp1 = _mm_shuffle_ps(tmp2, tmp1, _MM_SHUFFLE(2, 0, 2, 0));
    tmp1 = _mm_div_ps(tmp1, det);
    tmp2 = _mm_set_ss(1.0f);
    dist = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 1, 1, 1));
    return (_mm_movemask_ps(_mm_cmpgt_ps(tmp1, tmp2)) == 14);
#else
    float det = VectorDot3f(edge0, pvec);
    float u = VectorDot3f(tvec, pvec);
    float v = VectorDot3f(ray.dir, qvec);
    float t = VectorDot3f(edge1, qvec);
    u /= det;
    v /= det;
    t /= det;
    dist = VectorSplat(t);
    return u >= 0.0f && v >= 0.0f && t >= 0.0f && u + v <= 1.0f;
#endif // NFE_USE_SSE
}

template<> NFCOMMON_API
bool Intersect(const Ray& ray, const Triangle& tri)
{
    Vector unusedDist;
    return RayTriangleIntersectInline(ray, tri, unusedDist);
}

template<> NFCOMMON_API
bool Intersect(const Ray& ray, const Triangle& tri, Vector& dist)
{
    return RayTriangleIntersectInline(ray, tri, dist);
}


// Ray-Sphere intersection functions ==============================================================

NFE_INLINE bool RaySphereIntersectInline(const Ray& ray, const Sphere& sphere, Vector& dist)
{
    Vector d = sphere.origin - ray.origin;
    float v = VectorDot3f(ray.dir, d);
    float det = sphere.r * sphere.r - VectorDot3f(d, d) + v * v;

    if (det > 0.0f)
    {
        dist = VectorSplat(v - sqrtf(det));
        return true;
    }
    return false;
}

template<> NFCOMMON_API
bool Intersect(const Ray& ray, const Sphere& sphere)
{
    Vector unusedDist;

    return RaySphereIntersectInline(ray, sphere, unusedDist);
}

template<> NFCOMMON_API
bool Intersect(const Ray& ray, const Sphere& sphere, Vector& dist)
{
    return RaySphereIntersectInline(ray, sphere, dist);
}

} // namespace Math
} // namespace NFE
