#pragma once

#include "Texture.h"

namespace NFE {
namespace RT {

class MixTexture : public ITexture
{
    NFE_DECLARE_POLYMORPHIC_CLASS(MixTexture)

public:
    NFE_RAYTRACER_API MixTexture(const TexturePtr& textureA, const TexturePtr& textureB, const TexturePtr& textureMask);

    virtual const char* GetName() const override;
    virtual const Math::Vec4f Evaluate(const Math::Vec4f& coords) const override;
    virtual const Math::Vec4f Sample(const Math::Vec2f u, Math::Vec4f& outCoords, float* outPdf) const override;

private:
    TexturePtr mTextureA;
    TexturePtr mTextureB;
    TexturePtr mTextureMask;
};

} // namespace RT
} // namespace NFE
