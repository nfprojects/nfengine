#include "PCH.h"
#include "TraversalContext.h"
#include "Rendering/RenderingContext.h"

namespace NFE {
namespace RT {

using namespace Math;

void PacketTraversalContext::StoreIntersection(
    RayGroup& rayGroup,
    const RayPacketTypes::Float& t, const RayPacketTypes::Float& u, const RayPacketTypes::Float& v,
    const RayPacketTypes::FloatMask& mask,
    uint32 objectID, uint32 subObjectID) const
{
    const int intMask = mask.GetMask();

    const uint64 combinedObjectId = (uint64)objectID | ((uint64)subObjectID << 32u);

    HitPoint* hitPoints = context.hitPoints;

    if (intMask)
    {
        rayGroup.maxDistances = RayPacketTypes::Float::Select(rayGroup.maxDistances, t, mask);

        for (uint32 k = 0; k < RayPacketTypes::GroupSize; ++k)
        {
            if ((intMask >> k) & 1)
            {
                HitPoint& hitPointRef = hitPoints[rayGroup.rayOffsets[k]];

                hitPointRef.distance = t[k];
                hitPointRef.u = u[k];
                hitPointRef.v = v[k];
                hitPointRef.combinedObjectId = combinedObjectId;
            }
        }
    }
}

} // namespace RT
} // namespace NFE
