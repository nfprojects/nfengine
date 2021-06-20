#pragma once

#include "Texture.h"
#include "../Utils/Memory.h"
#include "../../Common/Math/HdrColor.hpp"

namespace NFE {
namespace RT {

class NFE_ALIGN(16) CheckerboardTexture
    : public ITexture
{
    NFE_DECLARE_POLYMORPHIC_CLASS(CheckerboardTexture)

public:
    NFE_RAYTRACER_API CheckerboardTexture();
    NFE_RAYTRACER_API CheckerboardTexture(const Math::Vec4f& colorA, const Math::Vec4f& colorB);

    virtual bool OnPropertyChanged(const Common::StringView propertyName) override;

    virtual const char* GetName() const override;
    virtual const Math::Vec4f Evaluate(const Math::Vec4f& coords) const override;
    virtual const Math::Vec4f Sample(const Math::Vec3f u, Math::Vec4f& outCoords, SampleDistortion distortion, float* outPdf) const override;

private:

    void UpdatePdf();

    Math::HdrColorRGBA mColorA;
    Math::HdrColorRGBA mColorB;

    // probability of sampling color A
    float mPdf;
};

} // namespace RT
} // namespace NFE
