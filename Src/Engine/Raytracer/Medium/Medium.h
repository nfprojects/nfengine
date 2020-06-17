#pragma once

#include "../RayLib.h"
#include "../Utils/Memory.h"
#include "../Color/RayColor.h"
#include "../../Common/Math/Ray.hpp"
#include "../../Common/Math/HdrColor.hpp"
#include "../../Common/Memory/Aligned.hpp"
#include "../../Common/Containers/SharedPtr.hpp"
#include "../../Common/Reflection/ReflectionClassDeclare.hpp"
#include "../../Common/Reflection/Object.hpp"

#include <memory>

namespace NFE {
namespace RT {

using TexturePtr = Common::SharedPtr<ITexture>;

struct RenderingContext;

// structure filled by Medium::Sample
struct MediumScatteringEvent
{
    RayColor radiance;             // radiance along the ray till the scattering event
    RayColor transmittance;        // medium transmittance between ray origin and scattering event

    float distance;             // distance along the ray where the scattering occurs
    Math::Vec4f direction;    // scattered ray direction
};

// generic medium interface
class IMedium : public IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(IMedium)
public:
    NFE_ALIGNED_CLASS(16)

    virtual ~IMedium();

    // sample medium scattering event
    virtual const RayColor Sample(const Math::Ray& ray, float minDistance, float maxDistance, MediumScatteringEvent& outScatteringEvent, RenderingContext& context) const = 0;

    // compute medium transmittance between two points
    virtual const RayColor Transmittance(const Math::Vec4f& startPoint, const Math::Vec4f& endPoint, RenderingContext& context) const = 0;
};

class HomogenousEmissiveMedium : public IMedium
{
    NFE_DECLARE_POLYMORPHIC_CLASS(HomogenousEmissiveMedium)
public:
    NFE_RAYTRACER_API HomogenousEmissiveMedium(const Math::HdrColorRGB emissionCoeff = 0.0f);
    NFE_RAYTRACER_API virtual const RayColor Sample(const Math::Ray& ray, float minDistance, float maxDistance, MediumScatteringEvent& outScatteringEvent, RenderingContext& context) const override;
    NFE_RAYTRACER_API virtual const RayColor Transmittance(const Math::Vec4f& startPoint, const Math::Vec4f& endPoint, RenderingContext& context) const override;

protected:
    Math::HdrColorRGB mEmissionCoeff;
};

class HomogenousAbsorptiveMedium : public IMedium
{
    NFE_DECLARE_POLYMORPHIC_CLASS(HomogenousAbsorptiveMedium)
public:
    NFE_RAYTRACER_API HomogenousAbsorptiveMedium(const Math::HdrColorRGB exctinctionCoeff = 0.5f);
    NFE_RAYTRACER_API virtual const RayColor Sample(const Math::Ray& ray, float minDistance, float maxDistance, MediumScatteringEvent& outScatteringEvent, RenderingContext& context) const override;
    NFE_RAYTRACER_API virtual const RayColor Transmittance(const Math::Vec4f& startPoint, const Math::Vec4f& endPoint, RenderingContext& context) const override;

protected:
    const RayColor Transmittance(const float distance, RenderingContext& ctx) const;

    Math::HdrColorRGB mExctinctionCoeff;
};

class HeterogeneousAbsorptiveMedium : public IMedium
{
    NFE_DECLARE_POLYMORPHIC_CLASS(HeterogeneousAbsorptiveMedium)
public:
    NFE_RAYTRACER_API HeterogeneousAbsorptiveMedium(const TexturePtr& densityTexture, const Math::HdrColorRGB exctinctionCoeff = Math::HdrColorRGB(1.0f, 1.0f, 1.0f));
    NFE_RAYTRACER_API virtual const RayColor Sample(const Math::Ray& ray, float minDistance, float maxDistance, MediumScatteringEvent& outScatteringEvent, RenderingContext& context) const override;
    NFE_RAYTRACER_API virtual const RayColor Transmittance(const Math::Vec4f& startPoint, const Math::Vec4f& endPoint, RenderingContext& context) const override;

protected:

    TexturePtr mDensityTexture;
    Math::HdrColorRGB mExctinctionCoeff;
    float mInvMaxDensity;
};

class HomogenousScatteringMedium : public HomogenousAbsorptiveMedium
{
    NFE_DECLARE_POLYMORPHIC_CLASS(HomogenousScatteringMedium)
public:
    NFE_RAYTRACER_API HomogenousScatteringMedium(const Math::HdrColorRGB exctinctionCoeff, const Math::HdrColorRGB scatteringAlbedo);
    NFE_RAYTRACER_API virtual const RayColor Sample(const Math::Ray& ray, float minDistance, float maxDistance, MediumScatteringEvent& outScatteringEvent, RenderingContext& context) const override;

private:
    Math::HdrColorRGB mScatteringAlbedo;
};

class HeterogeneousScatteringMedium : public HeterogeneousAbsorptiveMedium
{
    NFE_DECLARE_POLYMORPHIC_CLASS(HeterogeneousScatteringMedium)
public:
    NFE_RAYTRACER_API HeterogeneousScatteringMedium(const TexturePtr& densityTexture, const Math::HdrColorRGB exctinctionCoeff, const Math::HdrColorRGB scatteringAlbedo);
    NFE_RAYTRACER_API virtual const RayColor Sample(const Math::Ray& ray, float minDistance, float maxDistance, MediumScatteringEvent& outScatteringEvent, RenderingContext& context) const override;

protected:

    Math::HdrColorRGB mScatteringAlbedo;
};

} // namespace RT
} // namespace NFE
