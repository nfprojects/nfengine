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
    const RayPacketTypes::Ray& ray;
    SimdHitPoint& hitPoint;
    RenderingContext& context;
};

struct PacketTraversalContext
{
    RayPacket& ray;
    RenderingContext& context;

    void StoreIntersection(RayGroup& rayGroup, const RayPacketTypes::Float& t, const RayPacketTypes::Float& u, const RayPacketTypes::Float& v, const RayPacketTypes::FloatMask& mask, uint32 objectID, uint32 subObjectID = 0) const;
};

} // namespace RT
} // namespace NFE
