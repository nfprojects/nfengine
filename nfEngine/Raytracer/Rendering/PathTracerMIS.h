#pragma once

#include "Renderer.h"
#include "../Material/BSDF/BSDF.h"

namespace NFE {
namespace RT {

// Unidirectional path tracer
// Samples both BSDF and direct lighting
// Uses MIS (Multiple Importance Sampling)
class PathTracerMIS : public IRenderer
{
public:
    PathTracerMIS(const Scene& scene);

    virtual const char* GetName() const override;
    virtual const RayColor RenderPixel(const Math::Ray& ray, const RenderParam& param, RenderingContext& ctx) const override;

    // for debugging
    Math::Vector4 mLightSamplingWeight;
    Math::Vector4 mBSDFSamplingWeight;

private:

    struct PathState
    {
        uint32 depth = 0u;
        float lastPdfW = 1.0f;
        bool lastSpecular = true;
    };

    float GetLightPickingProbability(RenderingContext& context) const;

    // importance sample light sources
    const RayColor SampleLights(const ShadingData& shadingData, const PathState& pathState, RenderingContext& context, const float lightPickProbability) const;

    // importance sample single light source
    const RayColor SampleLight(const LightSceneObject* lightObject, const ShadingData& shadingData, const PathState& pathState, RenderingContext& context, const float lightPickProbability) const;

    // compute radiance from a hit local lights
    const RayColor EvaluateLight(const LightSceneObject* lightObject, const Math::Ray& ray, float dist, const IntersectionData& intersection, const PathState& pathState, RenderingContext& context, const float lightPickProbability) const;

    // compute radiance from global lights
    const RayColor EvaluateGlobalLights(const Math::Ray& ray, const PathState& pathState, RenderingContext& context, const float lightPickProbability) const;
};

} // namespace RT
} // namespace NFE
