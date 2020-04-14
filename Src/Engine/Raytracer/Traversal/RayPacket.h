#pragma once

#include "RayPacketTypes.h"
#include "../BVH/BVH.h"


namespace NFE {
namespace RT {

// maximum number of rays in ray stream
// TODO experiment with this value
static const uint32 MaxRayPacketSize = 4096;

struct NFE_ALIGN(4) ImageLocationInfo
{
    uint16 x;
    uint16 y;

    ImageLocationInfo() = default;
    NFE_FORCE_INLINE ImageLocationInfo(uint32 x, uint32 y)
        : x((uint16)x)
        , y((uint16)y)
    { }
};

struct RayGroup
{
    RayPacketTypes::Ray rays[2];
    RayPacketTypes::Float maxDistances;
    RayPacketTypes::Uint32 rayOffsets;
};

// packet of coherent rays
struct RayPacket
{
    static constexpr uint32 GroupSize = RayPacketTypes::GroupSize;
    static constexpr uint32 MaxNumGroups = MaxRayPacketSize / GroupSize;

    RayGroup groups[MaxNumGroups];

    // rays influence on the image (e.g. 1.0 for primary rays)
    RayPacketTypes::Vec3f rayWeights[MaxNumGroups];

    // corresponding image pixels
    ImageLocationInfo imageLocations[MaxRayPacketSize];

    // number of rays (not groups!)
    uint32 numRays;

    NFE_FORCE_INLINE RayPacket()
        : numRays(0)
    { }

    NFE_FORCE_INLINE uint32 GetNumGroups() const
    {
        return (numRays + GroupSize - 1) / GroupSize;
    }

    NFE_FORCE_INLINE void PushRay(const Math::Ray& ray, const Math::Vec4f& weight, const ImageLocationInfo& location)
    {
        NFE_ASSERT(numRays < MaxRayPacketSize);

        const uint32 groupIndex = numRays / GroupSize;
        const uint32 rayIndex = numRays % GroupSize;

        RayGroup& group = groups[groupIndex];
        group.rays[0].dir.x[rayIndex] = ray.dir.x;
        group.rays[0].dir.y[rayIndex] = ray.dir.y;
        group.rays[0].dir.z[rayIndex] = ray.dir.z;
        group.rays[0].origin.x[rayIndex] = ray.origin.x;
        group.rays[0].origin.y[rayIndex] = ray.origin.y;
        group.rays[0].origin.z[rayIndex] = ray.origin.z;
        group.rays[0].invDir.x[rayIndex] = ray.invDir.x;
        group.rays[0].invDir.y[rayIndex] = ray.invDir.y;
        group.rays[0].invDir.z[rayIndex] = ray.invDir.z;
        group.maxDistances[rayIndex] = FLT_MAX;
        group.rayOffsets[rayIndex] = numRays;

        rayWeights[groupIndex].x[rayIndex] = weight.x;
        rayWeights[groupIndex].y[rayIndex] = weight.y;
        rayWeights[groupIndex].z[rayIndex] = weight.z;

        imageLocations[numRays] = location;

        numRays++;
    }

    void PushRays(const RayPacketTypes::Ray& rays, const RayPacketTypes::Vec3f& weights, const ImageLocationInfo* locations)
    {
        NFE_ASSERT((numRays < MaxRayPacketSize) && (numRays % GroupSize == 0));

        RayGroup& group = groups[numRays / GroupSize];
        group.rays[0] = rays;
        group.maxDistances = RayPacketTypes::Float(FLT_MAX);
        group.rayOffsets = RayPacketTypes::Uint32::Iota(numRays);

        rayWeights[numRays / GroupSize] = weights;

        // Note: this should be replaced with a single MOVUPS instruction
        memcpy(imageLocations + numRays, locations, sizeof(ImageLocationInfo) * GroupSize);

        numRays += GroupSize;
    }

    NFE_FORCE_INLINE void Clear()
    {
        numRays = 0;
    }
};


} // namespace RT
} // namespace NFE
