#pragma once

#include "Vec3x8f.hpp"
#include "Ray.hpp"
#include "../Utils/BitUtils.hpp"

namespace NFE {
namespace Math {

/**
 * 4 rays (SIMD version).
 */
class NFE_ALIGN(32) Ray_Simd8 : public Common::Aligned<32>
{
public:
    Vec3x8f dir;
    Vec3x8f origin;
    Vec3x8f invDir; 

    Ray_Simd8() = default;
    Ray_Simd8(const Ray_Simd8&) = default;
    Ray_Simd8& operator = (const Ray_Simd8&) = default;

    // splat single ray
    NFE_FORCE_INLINE explicit Ray_Simd8(const Ray& ray)
        : dir(ray.dir)
        , origin(ray.origin)
        , invDir(ray.invDir)
    {
    }

    // build SIMD ray from 8 rays
    NFE_FORCE_INLINE Ray_Simd8(const Ray& ray0, const Ray& ray1, const Ray& ray2, const Ray& ray3,
                              const Ray& ray4, const Ray& ray5, const Ray& ray6, const Ray& ray7)
        : dir(ray0.dir, ray1.dir, ray2.dir, ray3.dir, ray4.dir, ray5.dir, ray6.dir, ray7.dir)
        , origin(ray0.origin, ray1.origin, ray2.origin, ray3.origin, ray4.origin, ray5.origin, ray6.origin, ray7.origin)
        , invDir(ray0.invDir, ray1.invDir, ray2.invDir, ray3.invDir, ray4.invDir, ray5.invDir, ray6.invDir, ray7.invDir)
    {
    }

    NFE_FORCE_INLINE Ray_Simd8(const Vec3x8f& origin, const Vec3x8f& dir)
        : dir(dir.Normalized())
        , origin(origin)
    {
        invDir = Vec3x8f::FastReciprocal(dir);
    }

    // return rays octant if all the rays are in the same on
    // otherwise, returns 0xFFFFFFFF
    NFE_FORCE_INLINE uint32 GetOctant() const
    {
        const size_t countX = Common::BitUtils<uint32>::CountBits(dir.x.GetSignMask());
        const size_t countY = Common::BitUtils<uint32>::CountBits(dir.y.GetSignMask());
        const size_t countZ = Common::BitUtils<uint32>::CountBits(dir.z.GetSignMask());

        const uint32 xPart[9] = { 0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, 1u << 0u };
        const uint32 yPart[9] = { 0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, 1u << 1u };
        const uint32 zPart[9] = { 0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, 1u << 2u };

        return xPart[countX] | yPart[countY] | zPart[countZ];
    }
};

} // namespace Math
} // namespace NFE
