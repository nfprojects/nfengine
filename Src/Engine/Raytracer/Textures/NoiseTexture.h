#pragma once

#include "Texture.h"
#include "../Utils/Memory.h"
#include "../../Common/Math/HdrColor.hpp"

namespace NFE {
namespace RT {

// 2D simplex noise texture
class NFE_ALIGN(16) NoiseTexture
    : public ITexture
{
    NFE_DECLARE_POLYMORPHIC_CLASS(NoiseTexture)

public:
    NFE_RAYTRACER_API NoiseTexture(
        const Math::Vec4f& colorA = Math::Vec4f(0.0f),
        const Math::Vec4f& colorB = Math::Vec4f(1.0f),
        const uint32 numOctaves = 1u);

    virtual const char* GetName() const override;
    virtual const Math::Vec4f Evaluate(const Math::Vec4f& coords) const override;
    virtual const Math::Vec4f Sample(const Math::Vec3f u, Math::Vec4f& outCoords, SampleDistortion distortion, float* outPdf) const override;

    float EvaluateInternal(const Math::Vec4f& coords) const;

private:
    Math::HdrColorRGBA mColorA;
    Math::HdrColorRGBA mColorB;
    uint32 mNumOctaves;
};

} // namespace RT
} // namespace NFE
