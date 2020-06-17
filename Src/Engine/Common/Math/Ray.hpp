#pragma once

#include "Math.hpp"
#include "Vec4f.hpp"


namespace NFE {
namespace Math {

/**
 * Single ray (non-SIMD).
 */
class NFE_ALIGN(16) Ray
{
public:
    NFE_ALIGNED_CLASS(16)

    Vec4f origin;
    Vec4f dir;
    Vec4f invDir;
    Vec4f originDivDir;

    NFE_FORCE_INLINE Ray() = default;

    NFE_FORCE_INLINE Ray(const Vec4f& origin, const Vec4f& direction)
        : origin(origin)
    {
        dir = direction.InvNormalized(invDir);
        originDivDir = origin * invDir;
    }

    // same as constructor, but direction must be already normalized
    NFE_FORCE_INLINE static const Ray BuildUnsafe(const Vec4f& origin, const Vec4f& direction)
    {
        Ray ray;
        ray.origin = origin;
        ray.dir = direction;
        ray.invDir = Vec4f::Reciprocal(direction);
        ray.originDivDir = origin * ray.invDir;
        return ray;
    }

    NFE_FORCE_INLINE const Vec4f GetAtDistance(const float t) const
    {
        return Vec4f::MulAndAdd(dir, t, origin);
    }

    NFE_FORCE_INLINE bool IsValid() const
    {
        return origin.IsValid() && dir.IsValid();
    }
};

class RayBoxSegment
{
public:
    float nearDist;
    float farDist;
};


} // namespace Math
} // namespace NFE
