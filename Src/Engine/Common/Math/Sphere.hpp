#pragma once

#include "Vec4f.hpp"


namespace NFE {
namespace Math {

/**
 * Sphere
 */
class NFE_ALIGN(16) Sphere : public Common::Aligned<16>
{
public:
    Vec4f origin; //< Sphere center
    float r; //< Sphere radius

    NFE_FORCE_INLINE Sphere()
        : origin(), r()
    {}

    NFE_FORCE_INLINE Sphere(const Vec4f& origin, float r)
        : origin(origin), r(r)
    {}

    NFE_FORCE_INLINE float SupportVertex(const Vec4f& dir) const
    {
        Vec4f pos = origin + r * dir;
        return Vec4f::Dot3(dir, pos);
    }
};


} // namespace Math
} // namespace NFE
