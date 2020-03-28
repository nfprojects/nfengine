#pragma once

#include "HitPoint.h"

namespace NFE {
namespace RT {

struct SingleTraversalContext
{
    const Math::Ray& ray;
    HitPoint& hitPoint;
    RenderingContext& context;
};

struct SimdTraversalContext
{
    const Math::Ray_Simd8& ray;
    HitPoint_Simd8& hitPoint;
    RenderingContext& context;
};

struct PacketTraversalContext
{
    RayPacket& ray;
    RenderingContext& context;

    void StoreIntersection(RayGroup& rayGroup, const Math::Vec8f& t, const Math::Vec8f& u, const Math::Vec8f& v, const Math::VecBool8f& mask, uint32 objectID, uint32 subObjectID = 0) const;
};

} // namespace RT
} // namespace NFE
