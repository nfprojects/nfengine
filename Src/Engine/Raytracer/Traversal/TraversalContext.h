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

    void StoreIntersection(RayGroup& rayGroup, const Math::Vector8& t, const Math::Vector8& u, const Math::Vector8& v, const Math::VectorBool8& mask, uint32 objectID, uint32 subObjectID = 0) const;
};

} // namespace RT
} // namespace NFE
