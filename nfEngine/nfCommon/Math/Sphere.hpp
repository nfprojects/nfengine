/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Sphere class declarations.
 */

#pragma once

#include "Math.hpp"
#include "Vector4.hpp"

namespace NFE {
namespace Math {

/**
 * Sphere
 */
class NFE_ALIGN(16) Sphere
{
public:
    Vector4 origin; //< Sphere center
    float r; //< Sphere radius

    Sphere() : origin(), r() {}
    explicit Sphere(const Vector4& origin, float r) : origin(origin), r(r) {}

    /**
     * Construct a sphere intersecting four points.
     */
    static bool ConstructFromPoints(const Vector4& p1, const Vector4& p2, const Vector4& p3, const Vector4& p4, Sphere& outSphere);

    NFE_INLINE float SupportVertex(const Vector4& dir) const
    {
        Vector4 pos = origin + r * dir;
        return Vector4::Dot3(dir, pos);
    }
};

} // namespace Math
} // namespace NFE
