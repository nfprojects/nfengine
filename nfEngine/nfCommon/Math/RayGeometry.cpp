/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Ray intersection functions.
 */

#include "PCH.hpp"
#include "Geometry.hpp"
#include "Box.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"
#include "Ray.hpp"

namespace NFE {
namespace Math {

// Ray-Box intersection functions =================================================================

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
