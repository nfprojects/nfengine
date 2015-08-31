/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Sphere class definition.
 */

#pragma once

#include "Math.hpp"
#include "Vector.hpp"

namespace NFE {
namespace Math {

/**
 * Sphere
 */
class NFE_ALIGN16 Sphere
{
public:
    Vector origin; //< Sphere center
    float r; //< Sphere radius

    NFE_INLINE Sphere() : origin(), r() {}
    NFE_INLINE Sphere(const Vector& origin, float r) : origin(origin), r(r) {}

    NFE_INLINE float SupportVertex(const Vector& dir) const
    {
        Vector pos = origin + r * dir;
        return VectorDot3(dir, pos).f[0];
    }
};

} // namespace Math
} // namespace NFE
