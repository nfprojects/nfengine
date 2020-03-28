#include "PCH.h"
#include "TraversalContext.h"
#include "Rendering/RenderingContext.h"

namespace NFE {
namespace RT {

using namespace Math;

void PacketTraversalContext::StoreIntersection(RayGroup& rayGroup, const Vec8f& t, const Vec8f& u, const Vec8f& v, const VecBool8f& mask, uint32 objectID, uint32 subObjectID) const
{
    const int intMask = mask.GetMask();

    const uint64 combinedObjectId = (uint64)objectID | ((uint64)subObjectID << 32u);

    HitPoint* hitPoints = context.hitPoints;

    if (intMask)
    {
        rayGroup.maxDistances = Vec8f::Select(rayGroup.maxDistances, t, mask);

        for (uint32 k = 0; k < 8; ++k)
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
