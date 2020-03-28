#include "PCH.h"
#include "Traversal_Packet.h"

namespace NFE {
namespace RT {

using namespace Math;

uint32 RemoveMissedGroups(RenderingContext& context, uint32 numGroups)
{
    /*
    uint32 i = 0;
    while (i < numGroups)
    {
        if (context.activeRaysMask[i])
        {
            i++;
        }
        else
        {
            numGroups--;
            std::swap(context.activeGroupsIndices[i], context.activeGroupsIndices[numGroups]);
            std::swap(context.activeRaysMask[i], context.activeRaysMask[numGroups]);
        }
    }

    return numGroups;
    */

    for (uint32 i = 0; ; )
    {
        // skip in-place hits at beginning
        while (context.activeRaysMask[i])
        {
            i++;
            if (i == numGroups)
            {
                return numGroups;
            }
        }

        // skip in-place misses at end
        uint8 mask;
        do
        {
            numGroups--;
            if (i == numGroups)
            {
                return numGroups;
            }
            mask = context.activeRaysMask[numGroups];
        } while (!mask);

        std::swap(context.activeGroupsIndices[i], context.activeGroupsIndices[numGroups]);
        context.activeRaysMask[i] = mask;
    }
}

// TODO this is ugly AF, but actually works
static void SwapRays(RenderingContext& context, uint32 a, uint32 b, uint32 traversalDepth)
{
    RayGroup& groupA = context.rayPacket.groups[context.activeGroupsIndices[a / RayPacket::RaysPerGroup]];
    RayGroup& groupB = context.rayPacket.groups[context.activeGroupsIndices[b / RayPacket::RaysPerGroup]];

    std::swap(groupA.rays[traversalDepth].dir.x[a % 8], groupB.rays[traversalDepth].dir.x[b % RayPacket::RaysPerGroup]);
    std::swap(groupA.rays[traversalDepth].dir.y[a % 8], groupB.rays[traversalDepth].dir.y[b % RayPacket::RaysPerGroup]);
    std::swap(groupA.rays[traversalDepth].dir.z[a % 8], groupB.rays[traversalDepth].dir.z[b % RayPacket::RaysPerGroup]);

    std::swap(groupA.rays[traversalDepth].origin.x[a % 8], groupB.rays[traversalDepth].origin.x[b % RayPacket::RaysPerGroup]);
    std::swap(groupA.rays[traversalDepth].origin.y[a % 8], groupB.rays[traversalDepth].origin.y[b % RayPacket::RaysPerGroup]);
    std::swap(groupA.rays[traversalDepth].origin.z[a % 8], groupB.rays[traversalDepth].origin.z[b % RayPacket::RaysPerGroup]);

    std::swap(groupA.rays[traversalDepth].invDir.x[a % 8], groupB.rays[traversalDepth].invDir.x[b % RayPacket::RaysPerGroup]);
    std::swap(groupA.rays[traversalDepth].invDir.y[a % 8], groupB.rays[traversalDepth].invDir.y[b % RayPacket::RaysPerGroup]);
    std::swap(groupA.rays[traversalDepth].invDir.z[a % 8], groupB.rays[traversalDepth].invDir.z[b % RayPacket::RaysPerGroup]);

    std::swap(groupA.maxDistances[a % 8], groupB.maxDistances[b % 8]);

    std::swap(groupA.rayOffsets[a % 8], groupB.rayOffsets[b % 8]);
}

static void SwapBits(uint8& a, uint8& b, uint32 indexA, uint32 indexB)
{
    const uint8 bitA = (a >> indexA) & 1;
    const uint8 bitB = (b >> indexB) & 1;
    a ^= (-bitB ^ a) & (1UL << indexA);
    b ^= (-bitA ^ b) & (1UL << indexB);
}

void ReorderRays(RenderingContext& context, uint32 numGroups, uint32 traversalDepth)
{
    uint32 numRays = numGroups * RayPacket::RaysPerGroup;
    uint32 i = 0;
    while (i < numRays)
    {
        const uint32 groupIndex = i / RayPacket::RaysPerGroup;
        const uint32 rayIndex = i % RayPacket::RaysPerGroup;

        if (context.activeRaysMask[groupIndex] & (1 << rayIndex))
        {
            i++;
        }
        else
        {
            numRays--;
            SwapRays(context, i, numRays, traversalDepth);
            SwapBits(context.activeRaysMask[i / 8], context.activeRaysMask[numRays / 8], i % 8, numRays % 8);
        }
    }
}

uint32 TestRayPacket(RayPacket& packet, uint32 numGroups, const BVH::Node& node, RenderingContext& context, uint32 traversalDepth)
{
    Vec8f distance;

    uint32 raysHit = 0;
    uint32 i = 0;

    const Math::Box_Simd8 box = node.GetBox_Simd8();

    // unrolled version of the loop below
    while (i + 4 <= numGroups)
    {
        const RayGroup& rayGroupA = packet.groups[context.activeGroupsIndices[i + 0]];
        const RayGroup& rayGroupB = packet.groups[context.activeGroupsIndices[i + 1]];
        const RayGroup& rayGroupC = packet.groups[context.activeGroupsIndices[i + 2]];
        const RayGroup& rayGroupD = packet.groups[context.activeGroupsIndices[i + 3]];

        const Ray_Simd8& raysA = rayGroupA.rays[traversalDepth];
        const Ray_Simd8& raysB = rayGroupB.rays[traversalDepth];
        const Ray_Simd8& raysC = rayGroupC.rays[traversalDepth];
        const Ray_Simd8& raysD = rayGroupD.rays[traversalDepth];

        const Vec3x8f rayOriginDivDirA = raysA.origin * raysA.invDir;
        const Vec3x8f rayOriginDivDirB = raysB.origin * raysB.invDir;
        const Vec3x8f rayOriginDivDirC = raysC.origin * raysC.invDir;
        const Vec3x8f rayOriginDivDirD = raysD.origin * raysD.invDir;

        const VecBool8f maskA = Intersect_BoxRay_Simd8(raysA.invDir, rayOriginDivDirA, box, rayGroupA.maxDistances, distance);
        const VecBool8f maskB = Intersect_BoxRay_Simd8(raysB.invDir, rayOriginDivDirB, box, rayGroupB.maxDistances, distance);
        const VecBool8f maskC = Intersect_BoxRay_Simd8(raysC.invDir, rayOriginDivDirC, box, rayGroupC.maxDistances, distance);
        const VecBool8f maskD = Intersect_BoxRay_Simd8(raysD.invDir, rayOriginDivDirD, box, rayGroupD.maxDistances, distance);

        const uint32 intMaskA = maskA.GetMask();
        const uint32 intMaskB = maskB.GetMask();
        const uint32 intMaskC = maskC.GetMask();
        const uint32 intMaskD = maskD.GetMask();

        const uint32 intMaskCombined = (intMaskA | (intMaskB << 8u)) | ((intMaskC << 16u) | (intMaskD << 24u));
        *reinterpret_cast<uint32*>(context.activeRaysMask + i) = intMaskCombined;
        raysHit += (uint32)Common::BitUtils<uint32>::CountBits(intMaskCombined);

        i += 4;
    }

    for (; i < numGroups; ++i)
    {
        const RayGroup& rayGroup = packet.groups[context.activeGroupsIndices[i]];
        const Vec3x8f rayOriginDivDir = rayGroup.rays[traversalDepth].origin * rayGroup.rays[traversalDepth].invDir;

        const VecBool8f mask = Intersect_BoxRay_Simd8(rayGroup.rays[traversalDepth].invDir, rayOriginDivDir, box, rayGroup.maxDistances, distance);
        const uint32 intMask = mask.GetMask();
        context.activeRaysMask[i] = (uint8)intMask;
        raysHit += (uint32)Common::BitUtils<uint32>::CountBits(intMask);
    }

    return raysHit;
}

} // namespace RT
} // namespace NFE
