#pragma once

#include "Texture.h"
#include "../Utils/Memory.h"
#include "../../Common/Math/HdrColor.hpp"

namespace NFE {
namespace RT {

// constant color texture
class NFE_ALIGN(16) ConstTexture
    : public ITexture
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ConstTexture)

public:
    NFE_RAYTRACER_API ConstTexture(const Math::Vec4f& color = Math::Vec4f(1.0f));

    virtual const char* GetName() const override;
    virtual const Math::Vec4f Evaluate(const Math::Vec4f& coords) const override;
    virtual const Math::Vec4f Sample(const Math::Vec3f u, Math::Vec4f& outCoords, SampleDistortion distortion, float* outPdf) const override;

private:
    Math::HdrColorRGBA mColor;
};

} // namespace RT
} // namespace NFE
