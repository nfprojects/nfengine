#pragma once

#include "Counters.h"
#include "RendererContext.h"

#include "../Traversal/RayPacket.h"
#include "../Traversal/HitPoint.h"
#include "../Color/RayColor.h"
#include "../Sampling/GenericSampler.h"
#include "../../Common/Math/Random.hpp"
#include "../../Common/Containers/UniquePtr.hpp"
#include "../../Common/Memory/Aligned.hpp"
#include "../../Common/Reflection/ReflectionClassDeclare.hpp"
#include "../../Common/Reflection/ReflectionEnumMacros.hpp"

namespace NFE {
namespace RT {

using RendererContextPtr = Common::UniquePtr<IRendererContext>;

struct PixelBreakpoint
{
    uint32 x = UINT32_MAX;
    uint32 y = UINT32_MAX;
};

struct NFE_RAYTRACER_API SpectrumDebugData
{
    Common::DynArray<float> samples;

    void Clear();
    void Accumulate(const RayColor& rayColor, const Wavelength& wavelength);
};

/**
 * A structure with local (per-thread) data.
 * It's like a hub for all global params (read only) and local state (read write).
 */
struct NFE_ALIGN(64) RenderingContext : public Common::Aligned<64>
{
    NFE_MAKE_NONCOPYABLE(RenderingContext)

public:

    NFE_RAYTRACER_API RenderingContext();
    NFE_RAYTRACER_API RenderingContext(RenderingContext&& other) = default;

    const Camera* camera = nullptr;

    Wavelength wavelength;

    GenericSampler sampler;

    // per-thread pseudo-random number generator
    Math::Random randomGenerator;

    // renderer-specific context, can be null
    RendererContextPtr rendererContext;

    // global rendering parameters
    const RenderingParams* params = nullptr;

    // per-thread counters
    RayTracingCounters counters;

    // counters used in local ray traversal routines
    LocalCounters localCounters;

    // for motion blur sampling
    float time = 0.0f;

#ifndef NFE_CONFIGURATION_FINAL
    // optional path debugging data
    PathDebugData* pathDebugData = nullptr;
    // break on specific pixel (for debugging)
    PixelBreakpoint pixelBreakpoint;
#endif // NFE_CONFIGURATION_FINAL

    SpectrumDebugData* spectrumDebugData = nullptr;

    RayPacket rayPacket;

    HitPoint hitPoints[MaxRayPacketSize];

    // TODO separate stacks for scene and mesh
    RayPacketTypes::RayMaskType activeRaysMask[RayPacket::MaxNumGroups];
    uint16 activeGroupsIndices[RayPacket::MaxNumGroups];
};


} // namespace RT
} // namespace NFE
