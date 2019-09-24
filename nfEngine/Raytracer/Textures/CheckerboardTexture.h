#pragma once

#include "Texture.h"
#include "../Utils/Memory.h"

namespace NFE {
namespace RT {

class NFE_ALIGN(16) CheckerboardTexture
    : public ITexture
{
public:
    NFE_RAYTRACER_API CheckerboardTexture(const Math::Vector4& colorA, Math::Vector4& colorB);

    virtual const char* GetName() const override;
    virtual const Math::Vector4 Evaluate(const Math::Vector4& coords) const override;
    virtual const Math::Vector4 Sample(const Math::Float2 u, Math::Vector4& outCoords, float* outPdf) const override;

private:
    Math::Vector4 mColorA;
    Math::Vector4 mColorB;

    // probability of sampling color A
    float mPdf;
};

} // namespace RT
} // namespace NFE
