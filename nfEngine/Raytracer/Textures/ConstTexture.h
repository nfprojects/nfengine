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
    NFE_RAYTRACER_API ConstTexture(const Math::Vector4& color);

    virtual const char* GetName() const override;
    virtual const Math::Vector4 Evaluate(const Math::Vector4& coords) const override;
    virtual const Math::Vector4 Sample(const Math::Float2 u, Math::Vector4& outCoords, float* outPdf) const override;

private:
    Math::Vector4 mColor;
};

} // namespace RT
} // namespace NFE
