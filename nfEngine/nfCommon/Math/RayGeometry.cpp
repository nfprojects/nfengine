/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Ray intersection functions.
 */

#include "PCH.hpp"
#include "Geometry.hpp"
#include "Sphere.hpp"

namespace NFE {
namespace Math {

// Ray-Box intersection functions =================================================================

template<> NFCOMMON_API
bool Intersect(const Ray& ray, const Box& box)
{
    Vector4 unusedDist;
    return RayBoxIntersectInline(ray, box, unusedDist);
}

template<> NFCOMMON_API
bool Intersect(const Ray& ray, const Box& box, Vector4& dist)
{
    return RayBoxIntersectInline(ray, box, dist);
}


// Ray-Triangle intersection functions ============================================================

template<> NFCOMMON_API
bool Intersect(const Ray& ray, const Triangle& tri)
{
    Vector4 unusedDist;
    return RayTriangleIntersectInline(ray, tri, unusedDist);
}

template<> NFCOMMON_API
bool Intersect(const Ray& ray, const Triangle& tri, Vector4& dist)
{
    return RayTriangleIntersectInline(ray, tri, dist);
}


// Ray-Sphere intersection functions ==============================================================

NFE_INLINE bool RaySphereIntersectInline(const Ray& ray, const Sphere& sphere, Vector4& dist)
{
    Vector4 d = sphere.origin - ray.origin;
    float v = Vector4::Dot3(ray.dir, d);
    float det = sphere.r * sphere.r - Vector4::Dot3(d, d) + v * v;

    if (det > 0.0f)
    {
        dist = Vector4::Splat(v - sqrtf(det));
        return true;
    }
    return false;
}

template<> NFCOMMON_API
bool Intersect(const Ray& ray, const Sphere& sphere)
{
    Vector4 unusedDist;

    return RaySphereIntersectInline(ray, sphere, unusedDist);
}

template<> NFCOMMON_API
bool Intersect(const Ray& ray, const Sphere& sphere, Vector4& dist)
{
    return RaySphereIntersectInline(ray, sphere, dist);
}

} // namespace Math
} // namespace NFE
