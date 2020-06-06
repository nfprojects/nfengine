#pragma once

#include "Renderer.h"
#include "../Material/BSDF/BSDF.h"

namespace NFE {
namespace RT {

// Naive unidirectional path tracer
// Note: this renderer is unable to sample "delta" lights (point and directional lights) 
class PathTracer : public IRenderer
{
    NFE_DECLARE_POLYMORPHIC_CLASS(PathTracer)

public:
    PathTracer();

    virtual const RayColor RenderPixel(const Math::Ray& ray, const RenderParam& param, RenderingContext& ctx) const override;

private:

    // compute radiance from a hit local lights
    const RayColor EvaluateLight(const LightSceneObject* lightObject, const Math::Ray& ray, const IntersectionData& intersection, RenderingContext& context) const;

    // compute radiance from global lights
    const RayColor EvaluateGlobalLights(const Scene& scene, const Math::Ray& ray, RenderingContext& context) const;
};

} // namespace RT
} // namespace NFE
