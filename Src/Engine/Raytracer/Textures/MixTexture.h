#pragma once

#include "Texture.h"

namespace NFE {
namespace RT {

class MixTexture : public ITexture
{
public:
    NFE_RAYTRACER_API MixTexture(const TexturePtr& textureA, const TexturePtr& textureB, const TexturePtr& textureMask);

    virtual const char* GetName() const override;
    virtual const Math::Vector4 Evaluate(const Math::Vector4& coords) const override;
    virtual const Math::Vector4 Sample(const Math::Float2 u, Math::Vector4& outCoords, float* outPdf) const override;

private:
    TexturePtr mTextureA;
    TexturePtr mTextureB;
    TexturePtr mTextureMask;
};

} // namespace RT
} // namespace NFE
