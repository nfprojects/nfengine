#pragma once

#include "Texture.h"
#include "../Utils/Memory.h"

namespace NFE {
namespace RT {

// 2D simplex noise texture
class NFE_ALIGN(16) NoiseTexture
    : public ITexture
{
public:
    NFE_RAYTRACER_API NoiseTexture(const Math::Vector4& colorA, const Math::Vector4& colorB, const uint32 numOctaves = 1);

    virtual const char* GetName() const override;
    virtual const Math::Vector4 Evaluate(const Math::Vector4& coords) const override;
    virtual const Math::Vector4 Sample(const Math::Float2 u, Math::Vector4& outCoords, float* outPdf) const override;

    float EvaluateInternal(const Math::Vector4& coords) const;

private:
    Math::Vector4 mColorA;
    Math::Vector4 mColorB;
    uint32 mNumOctaves;
};

} // namespace RT
} // namespace NFE
