#pragma once

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

    NFE_FORCE_INLINE Sphere()
        : origin(), r()
    {}

    NFE_FORCE_INLINE Sphere(const Vector4& origin, float r)
        : origin(origin), r(r)
    {}

    NFE_FORCE_INLINE float SupportVertex(const Vector4& dir) const
    {
        Vector4 pos = origin + r * dir;
        return Vector4::Dot3(dir, pos);
    }
};


} // namespace Math
} // namespace NFE
