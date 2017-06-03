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
    Sphere() : origin(), r() {}
    explicit Sphere(const Vector& origin, float r) : origin(origin.ToFloat3()), r(r) {}
    explicit Sphere(const Float3& origin, float r) : origin(origin), r(r) {}

    /**
     * Construct a sphere intersecting four points.
     */
    static bool ConstructFromPoints(const Vector& p1, const Vector& p2, const Vector& p3, const Vector& p4, Sphere& outSphere);

    /**
     * Get sphere's origin.
     */
    Vector GetOrigin() const
    {
        return Vector(origin);
    }

    /**
     * Get sphere's radius.
     */
    float GetRadius() const
    {
        return r;
    }

    NFE_INLINE float SupportVertex(const Vector& dir) const
    {
        const Vector pos = GetOrigin() + r * dir;
        return Vector::Dot3(dir, pos);
    }

private:
    union
    {
        Vector data;

        struct
        {
            Float3 origin; //< Sphere center
            float r; //< Sphere radius
        };
    };
};

} // namespace Math
} // namespace NFE
