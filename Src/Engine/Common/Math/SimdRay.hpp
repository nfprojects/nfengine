#pragma once

#include "Ray.hpp"
#include "../Utils/BitUtils.hpp"

namespace NFE {
namespace Math {

template<typename VecType>
class NFE_ALIGN(alignof(VecType)) SimdRay
{
public:
    NFE_ALIGNED_CLASS(alignof(VecType))

    VecType dir;
    VecType origin;
    VecType invDir; 

    SimdRay() = default;
    SimdRay(const SimdRay&) = default;
    SimdRay& operator = (const SimdRay&) = default;

    // splat single ray
    NFE_FORCE_INLINE explicit SimdRay(const Ray& ray)
        : dir(ray.dir)
        , origin(ray.origin)
        , invDir(ray.invDir)
    {
    }

    NFE_FORCE_INLINE SimdRay(const VecType& origin, const VecType& dir)
        : dir(dir.Normalized())
        , origin(origin)
    {
        invDir = VecType::FastReciprocal(dir);
    }

    // return rays octant if all the rays are in the same on
    // otherwise, returns 0xFFFFFFFF
    NFE_FORCE_INLINE uint32 GetOctant() const
    {
        const uint32 countX = Common::BitUtils<uint32>::CountBits(dir.x.GetSignMask());
        const uint32 countY = Common::BitUtils<uint32>::CountBits(dir.y.GetSignMask());
        const uint32 countZ = Common::BitUtils<uint32>::CountBits(dir.z.GetSignMask());

        const uint32 xPart[9] = { 0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, 1u << 0u };
        const uint32 yPart[9] = { 0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, 1u << 1u };
        const uint32 zPart[9] = { 0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, 1u << 2u };

        return xPart[countX] | yPart[countY] | zPart[countZ];
    }
};

} // namespace Math
} // namespace NFE
