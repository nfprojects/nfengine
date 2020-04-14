#pragma once

#include "RayPacket.h"
#include "../../Common/Math/Vec8i.hpp"

namespace NFE {
namespace RT {

// Ray-scene intersection data (non-SIMD)
struct HitPoint
{
    union
    {
        struct
        {
            uint32 objectId;
            uint32 subObjectId;
        };

        uint64 combinedObjectId;
    };

    float distance;

    union
    {
        struct
        {
            float u;
            float v;
        };

        uint64 combinedUV;
    };

    static constexpr uint32 InvalidObject = UINT32_MAX;
    static constexpr float DefaultDistance = std::numeric_limits<float>::infinity();

    NFE_FORCE_INLINE HitPoint()
    {
        Reset();
    }

    NFE_FORCE_INLINE void Reset()
    {
        objectId = InvalidObject;
        distance = DefaultDistance;
        combinedUV = 0;
    }

    NFE_FORCE_INLINE void Set(float newDistance, uint32 newObjectId, uint32 newSubObjectId)
    {
        distance = newDistance;
        objectId = newObjectId;
        subObjectId = newSubObjectId;
    }

    // optimization: perform single 64-bit write instead of two 32-bit writes
    NFE_FORCE_INLINE void Set(float newDistance, uint32 newObjectId)
    {
        distance = newDistance;
        combinedObjectId = newObjectId;
    }
};

// Ray-scene intersection data
struct SimdHitPoint
{
    RayPacketTypes::Float distance;
    RayPacketTypes::Float u;
    RayPacketTypes::Float v;

    // TODO Uint32
    RayPacketTypes::Int32 objectId;
    RayPacketTypes::Int32 subObjectId;

    NFE_FORCE_INLINE SimdHitPoint()
        : distance(FLT_MAX)
        , objectId(HitPoint::InvalidObject)
    {}

    // extract single hit point
    NFE_FORCE_INLINE HitPoint Get(uint32 i) const
    {
        NFE_ASSERT(i < RayPacketTypes::GroupSize);

        HitPoint result;
        result.distance = distance[i];
        result.u = u[i];
        result.v = v[i];
        result.objectId = objectId[i];
        result.subObjectId = subObjectId[i];
        return result;
    }
};

} // namespace RT
} // namespace NFE
