#pragma once

#include "Light.h"
#include "../../../nfCommon/Containers/SharedPtr.hpp"

namespace NFE {
namespace RT {

class ITexture;
using TexturePtr = Common::SharedPtr<ITexture>;

class BackgroundLight : public ILight
{
    NFE_DECLARE_POLYMORPHIC_CLASS(BackgroundLight);

public:
    NFE_RAYTRACER_API BackgroundLight();
    NFE_RAYTRACER_API BackgroundLight(const Math::HdrColorRGB& color);

    TexturePtr mTexture = nullptr;

    virtual const Math::Box GetBoundingBox() const override;
    virtual const RayColor Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const override;
    virtual const RayColor GetRadiance(const RadianceParam& param, float* outDirectPdfA, float* outEmissionPdfW) const override;
    virtual const RayColor Emit(const EmitParam& param, EmitResult& outResult) const override;
    virtual Flags GetFlags() const override final;

    const RayColor GetBackgroundColor(const Math::Vector4& dir, const Wavelength& wavelength) const;
};

} // namespace RT
} // namespace NFE
