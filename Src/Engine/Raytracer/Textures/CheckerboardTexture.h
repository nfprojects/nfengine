#pragma once

#include "Texture.h"
#include "../Utils/Memory.h"

namespace NFE {
namespace RT {

class NFE_ALIGN(16) CheckerboardTexture
    : public ITexture
{
public:
    NFE_RAYTRACER_API CheckerboardTexture(const Math::Vec4f& colorA, const Math::Vec4f& colorB);

    virtual const char* GetName() const override;
    virtual const Math::Vec4f Evaluate(const Math::Vec4f& coords) const override;
    virtual const Math::Vec4f Sample(const Math::Vec2f u, Math::Vec4f& outCoords, float* outPdf) const override;

private:
    Math::Vec4f mColorA;
    Math::Vec4f mColorB;

    // probability of sampling color A
    float mPdf;
};

} // namespace RT
} // namespace NFE
