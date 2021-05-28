#pragma once

#include "Renderer.h"
#include "../Material/BSDF/BSDF.h"

namespace NFE {
namespace RT {

// Naive unidirectional light tracer
// Traces random light paths starting from light surface
// After hitting a geometry, connects to camera and splats the contribution onto film
// Note: This renderer is unable to render specular materials viewed by the camera directly
class LightTracer : public IRenderer
{
    NFE_DECLARE_POLYMORPHIC_CLASS(LightTracer)

public:
    LightTracer();

    virtual const RayColor RenderPixel(const Math::Ray& ray, const RenderParam& param, RenderingContext& ctx) const override;

};

} // namespace RT
} // namespace NFE
