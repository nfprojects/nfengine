#pragma once

#include "Renderer.h"
#include "../Material/BSDF/BSDF.h"
#include "../../nfCommon/Math/LdrColor.hpp"

namespace NFE {
namespace RT {

// Unidirectional path tracer
// Samples both BSDF and direct lighting
// Uses MIS (Multiple Importance Sampling)
class PathTracerMIS : public IRenderer
{
    NFE_DECLARE_POLYMORPHIC_CLASS(PathTracerMIS);

public:
    PathTracerMIS();

    virtual const RayColor RenderPixel(const Math::Ray& ray, const RenderParam& param, RenderingContext& ctx) const override;

private:

    struct PathState
    {
        uint32 depth = 0u;
        float lastPdfW = 1.0f;
        bool lastSpecular = true;
    };

    float GetLightPickingProbability(const Scene& scene, RenderingContext& context) const;

    // importance sample light sources
    const RayColor SampleLights(const Scene& scene, const ShadingData& shadingData, const PathState& pathState, RenderingContext& context, const float lightPickProbability) const;

    // importance sample single light source
    const RayColor SampleLight(const Scene& scene, const LightSceneObject* lightObject, const ShadingData& shadingData, const PathState& pathState, RenderingContext& context, const float lightPickProbability) const;

    // compute radiance from a hit local lights
    const RayColor EvaluateLight(const LightSceneObject* lightObject, const Math::Ray& ray, float dist, const IntersectionData& intersection, const PathState& pathState, RenderingContext& context, const float lightPickProbability) const;

    // compute radiance from global lights
    const RayColor EvaluateGlobalLights(const Scene& scene, const Math::Ray& ray, const PathState& pathState, RenderingContext& context, const float lightPickProbability) const;

    // for debugging
    Math::LdrColorRGB lightSamplingWeight;
    Math::LdrColorRGB BSDFSamplingWeight;
};

} // namespace RT
} // namespace NFE
