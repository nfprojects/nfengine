#pragma once

#include "Texture.h"
#include "../Utils/Memory.h"

namespace NFE {
namespace RT {

// constant color texture
class NFE_ALIGN(16) ConstTexture
    : public ITexture
{
public:
    NFE_RAYTRACER_API ConstTexture(const Math::Vec4f& color);

    virtual const char* GetName() const override;
    virtual const Math::Vec4f Evaluate(const Math::Vec4f& coords) const override;
    virtual const Math::Vec4f Sample(const Math::Vec2f u, Math::Vec4f& outCoords, float* outPdf) const override;

private:
    Math::Vec4f mColor;
};

} // namespace RT
} // namespace NFE
