#pragma once

#include "../BVH/BVH.h"
#include "../../Common/Math/Ray.hpp"
#include "../../Common/Math/Simd8Ray.hpp"
#include "../../Common/Math/Vec8i.hpp"


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

struct NFE_ALIGN(32) RayGroup
{
    Math::Ray_Simd8 rays[2];
    Math::Vec8f maxDistances;
    Math::Vec8i rayOffsets;
};

// packet of coherent rays (8-SIMD version)
struct NFE_ALIGN(32) RayPacket
{
    static constexpr uint32 RaysPerGroup = 8;
    static constexpr uint32 MaxNumGroups = MaxRayPacketSize / RaysPerGroup;

    RayGroup groups[MaxNumGroups];

    // rays influence on the image (e.g. 1.0 for primary rays)
    Math::Vec3x8f rayWeights[MaxNumGroups];

    // corresponding image pixels
    ImageLocationInfo imageLocations[MaxRayPacketSize];

    // number of rays (not groups!)
    uint32 numRays;

    NFE_FORCE_INLINE RayPacket()
        : numRays(0)
    { }

    NFE_FORCE_INLINE uint32 GetNumGroups() const
    {
        return (numRays + RaysPerGroup - 1) / RaysPerGroup;
    }

    NFE_FORCE_INLINE void PushRay(const Math::Ray& ray, const Math::Vec4f& weight, const ImageLocationInfo& location)
    {
        NFE_ASSERT(numRays < MaxRayPacketSize);

        const uint32 groupIndex = numRays / RaysPerGroup;
        const uint32 rayIndex = numRays % RaysPerGroup;

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

    // TODO use non-temporal stores?
    NFE_FORCE_INLINE void PushRays(const Math::Ray_Simd8& rays, const Math::Vec3x8f& weights, const ImageLocationInfo* locations)
    {
        NFE_ASSERT((numRays < MaxRayPacketSize) && (numRays % RaysPerGroup == 0));

        RayGroup& group = groups[numRays / RaysPerGroup];
        group.rays[0] = rays;
        group.maxDistances = Math::VECTOR8_MAX;
        group.rayOffsets = Math::Vec8i(numRays) + Math::Vec8i(0, 1, 2, 3, 4, 5, 6, 7);

        rayWeights[numRays / RaysPerGroup] = weights;

        // Note: this should be replaced with a single MOVUPS instruction
        memcpy(imageLocations + numRays, locations, sizeof(ImageLocationInfo) * 8);

        numRays += RaysPerGroup;
    }

    NFE_FORCE_INLINE void Clear()
    {
        numRays = 0;
    }
};

/*
struct NFE_ALIGN(64) RayStream
{
    float rayOriginX[MaxRayPacketSize];
    float rayOriginY[MaxRayPacketSize];
    float rayOriginZ[MaxRayPacketSize];

    float rayDirX[MaxRayPacketSize];
    float rayDirY[MaxRayPacketSize];
    float rayDirZ[MaxRayPacketSize];

    float distance[MaxRayPacketSize];
};

struct NFE_ALIGN(64) RayStreamHitData
{
    float distance[MaxRayPacketSize];
    float u[MaxRayPacketSize];
    float v[MaxRayPacketSize];
    uint32 subObjectId[MaxRayPacketSize];
    uint32 objectId[MaxRayPacketSize];
};
*/

} // namespace RT
} // namespace NFE
