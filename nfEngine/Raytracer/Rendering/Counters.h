#pragma once

#include "../Raytracer.h"
#include "../Config.h"


namespace NFE {
namespace RT {


struct LocalCounters
{
#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
    uint32 numRayBoxTests;
    uint32 numPassedRayBoxTests;
    uint32 numRayTriangleTests;
    uint32 numPassedRayTriangleTests;
#endif // NFE_ENABLE_INTERSECTION_COUNTERS

    NFE_FORCE_INLINE LocalCounters()
    {
        Reset();
    }

    NFE_FORCE_INLINE void Reset()
    {
#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
        numRayBoxTests = 0;
        numPassedRayBoxTests = 0;
        numRayTriangleTests = 0;
        numPassedRayTriangleTests = 0;
#endif // NFE_ENABLE_INTERSECTION_COUNTERS
    }
};


struct RayTracingCounters
{
    uint64 numRays;
    uint64 numShadowRays;
    uint64 numShadowRaysHit;
    uint64 numPrimaryRays;

#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
    uint64 numRayBoxTests;
    uint64 numPassedRayBoxTests;
    uint64 numRayTriangleTests;
    uint64 numPassedRayTriangleTests;
#endif // NFE_ENABLE_INTERSECTION_COUNTERS


    NFE_FORCE_INLINE void Reset()
    {
        numRays = 0;
        numShadowRays = 0;
        numShadowRaysHit = 0;
        numPrimaryRays = 0;

#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
        numRayBoxTests = 0;
        numPassedRayBoxTests = 0;
        numRayTriangleTests = 0;
        numPassedRayTriangleTests = 0;
#endif // NFE_ENABLE_INTERSECTION_COUNTERS
    }


    NFE_FORCE_INLINE void Append(const LocalCounters& other)
    {
        NFE_UNUSED(other);
#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
        numRayBoxTests += other.numRayBoxTests;
        numPassedRayBoxTests += other.numPassedRayBoxTests;
        numRayTriangleTests += other.numRayTriangleTests;
        numPassedRayTriangleTests += other.numPassedRayTriangleTests;
#endif // NFE_ENABLE_INTERSECTION_COUNTERS
    }


    void Append(const RayTracingCounters& other)
    {
        numRays += other.numRays;
        numShadowRays += other.numShadowRays;
        numShadowRaysHit += other.numShadowRaysHit;
        numPrimaryRays += other.numPrimaryRays;

#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
        numRayBoxTests += other.numRayBoxTests;
        numPassedRayBoxTests += other.numPassedRayBoxTests;
        numRayTriangleTests += other.numRayTriangleTests;
        numPassedRayTriangleTests += other.numPassedRayTriangleTests;
#endif // NFE_ENABLE_INTERSECTION_COUNTERS
    }
};


} // namespace RT
} // namespace NFE
