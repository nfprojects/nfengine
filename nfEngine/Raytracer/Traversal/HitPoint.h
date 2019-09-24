#pragma once

#include "RayPacket.h"
#include "../../nfCommon/Math/VectorInt8.hpp"

namespace NFE {
namespace RT {

constexpr uint32 NFE_INVALID_OBJECT = UINT32_MAX;
constexpr uint32 NFE_LIGHT_OBJECT = 0xFFFFFFFE;

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
    float u;
    float v;

    static constexpr float DefaultDistance = std::numeric_limits<float>::infinity();

    NFE_FORCE_INLINE HitPoint()
        : objectId(NFE_INVALID_OBJECT)
        , distance(DefaultDistance)
    {}

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

// Ray-scene intersection data (SIMD-8)
struct NFE_ALIGN(32) HitPoint_Simd8
{
    Math::Vector8 distance;
    Math::Vector8 u;
    Math::Vector8 v;
    Math::VectorInt8 objectId;
    Math::VectorInt8 subObjectId;

    NFE_FORCE_INLINE HitPoint_Simd8()
        : distance(Math::VECTOR8_MAX)
        , objectId(NFE_INVALID_OBJECT)
    {}

    // extract single hit point
    NFE_FORCE_INLINE HitPoint Get(uint32 i) const
    {
        NFE_ASSERT(i < 8);

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
