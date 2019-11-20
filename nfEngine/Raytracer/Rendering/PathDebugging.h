#pragma once

#include "ShadingData.h"
#include "../Material/BSDF/BSDF.h"

namespace NFE {
namespace RT {


enum class PathTerminationReason
{
    None = 0,
    HitBackground,
    HitLight,
    Depth,
    Throughput,
    AttenuatedInMedium,
    NoSampledEvent,
    RussianRoulette,
};

enum class RaySource : uint8
{
    Eye = 0,
    Reflection,
    Refraction,
};

struct PathDebugData
{
    struct NFE_ALIGN(32) HitPointData
    {
        // ray traced
        Math::Vector4 rayOrigin;
        Math::Vector4 rayDir;

        HitPoint hitPoint;

        const IMedium* medium = nullptr;

        const ISceneObject* objectHit = nullptr;

        // evaluated world-space shading data
        ShadingData shadingData;

        // ray influence on the final pixel
        RayColor throughput;

        // evaluated ray color (excluding weight)
        Math::Vector4 color;

        RaySource raySource;

        BSDF::EventType bsdfEvent = BSDF::NullEvent;
    };

    PathTerminationReason terminationReason = PathTerminationReason::None;
    Common::DynArray<HitPointData> data;

    void Clear()
    {
        terminationReason = PathTerminationReason::None;
        data.Clear();
    }
};


} // namespace RT
} // namespace NFE
