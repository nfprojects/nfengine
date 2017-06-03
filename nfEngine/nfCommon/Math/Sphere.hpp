/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Sphere class declarations.
 */

#pragma once

#include "Math.hpp"
#include "Vector.hpp"

namespace NFE {
namespace Math {

/**
 * Sphere
 */
class NFE_ALIGN(16) Sphere
{
public:
    Vector origin; //< Sphere center
    float r; //< Sphere radius

    Sphere() : origin(), r() {}
    Sphere(const Vector& origin, float r) : origin(origin), r(r) {}

    /**
     * Construct a sphere intersecting four points.
     */
    static bool ConstructFromPoints(const Vector& p1, const Vector& p2, const Vector& p3, const Vector& p4, Sphere& outSphere);

    NFE_INLINE float SupportVertex(const Vector& dir) const
    {
        Vector pos = origin + r * dir;
        return Vector::Dot3(dir, pos);
    }
};

} // namespace Math
} // namespace NFE
