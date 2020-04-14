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
        RayPacketTypes::RayMaskType mask;
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

NFE_FORCE_INLINE
static void SwapRays(RenderingContext& context, uint32 a, uint32 b, uint32 traversalDepth)
{
    constexpr uint32 GroupSize = RayPacket::GroupSize;

    RayGroup& groupA = context.rayPacket.groups[context.activeGroupsIndices[a / GroupSize]];
    RayGroup& groupB = context.rayPacket.groups[context.activeGroupsIndices[b / GroupSize]];

    RayPacketTypes::Ray& raysA = groupA.rays[traversalDepth];
    RayPacketTypes::Ray& raysB = groupB.rays[traversalDepth];

    std::swap(raysA.dir.x[a % GroupSize], raysB.dir.x[b % GroupSize]);
    std::swap(raysA.dir.y[a % GroupSize], raysB.dir.y[b % GroupSize]);
    std::swap(raysA.dir.z[a % GroupSize], raysB.dir.z[b % GroupSize]);

    std::swap(raysA.origin.x[a % GroupSize], raysB.origin.x[b % GroupSize]);
    std::swap(raysA.origin.y[a % GroupSize], raysB.origin.y[b % GroupSize]);
    std::swap(raysA.origin.z[a % GroupSize], raysB.origin.z[b % GroupSize]);

    std::swap(raysA.invDir.x[a % GroupSize], raysB.invDir.x[b % GroupSize]);
    std::swap(raysA.invDir.y[a % GroupSize], raysB.invDir.y[b % GroupSize]);
    std::swap(raysA.invDir.z[a % GroupSize], raysB.invDir.z[b % GroupSize]);

    std::swap(groupA.maxDistances[a % GroupSize], groupB.maxDistances[b % GroupSize]);

    std::swap(groupA.rayOffsets[a % GroupSize], groupB.rayOffsets[b % GroupSize]);
}

template<typename T>
NFE_FORCE_INLINE
static void SwapBits(T& a, T& b, uint32 indexA, uint32 indexB)
{
    const T bitA = (a >> indexA) & 1;
    const T bitB = (b >> indexB) & 1;
    a ^= (-bitB ^ a) & (1UL << indexA);
    b ^= (-bitA ^ b) & (1UL << indexB);
}

void ReorderRays(RenderingContext& context, uint32 numGroups, uint32 traversalDepth)
{
    constexpr uint32 GroupSize = RayPacket::GroupSize;

    uint32 numRays = numGroups * RayPacket::GroupSize;
    uint32 i = 0;
    while (i < numRays)
    {
        const uint32 groupIndex = i / RayPacket::GroupSize;
        const uint32 rayIndex = i % RayPacket::GroupSize;

        if (context.activeRaysMask[groupIndex] & (1 << rayIndex))
        {
            i++;
        }
        else
        {
            numRays--;
            SwapRays(context, i, numRays, traversalDepth);
            SwapBits(context.activeRaysMask[i / GroupSize], context.activeRaysMask[numRays / GroupSize], i % GroupSize, numRays % GroupSize);
        }
    }
}

uint32 TestRayPacket(RayPacket& packet, uint32 numGroups, const BVH::Node& node, RenderingContext& context, uint32 traversalDepth)
{
    static_assert(8 * sizeof(RayPacketTypes::RayMaskType) >= RayPacketTypes::GroupSize, "Ray mask type is too small");

    RayPacketTypes::Float distance;

    uint32 raysHit = 0;

    const auto box = node.GetSimdBox<RayPacketTypes::Vec3f>();

    for (uint32 i = 0; i < numGroups; ++i)
    {
        const RayGroup& rayGroup = packet.groups[context.activeGroupsIndices[i]];
        const RayPacketTypes::Vec3f rayOriginDivDir = rayGroup.rays[traversalDepth].origin * rayGroup.rays[traversalDepth].invDir;
        const RayPacketTypes::Vec3f rayInvDir = rayGroup.rays[traversalDepth].invDir;

        const auto mask = Simd<RayPacketTypes::GroupSize>::Intersect_BoxRay(rayInvDir, rayOriginDivDir, box, rayGroup.maxDistances, distance);
        const auto intMask = mask.GetMask();
        context.activeRaysMask[i] = (RayPacketTypes::RayMaskType)intMask;
        raysHit += Common::BitUtils<uint32>::CountBits(intMask);
    }

    return raysHit;
}

} // namespace RT
} // namespace NFE
