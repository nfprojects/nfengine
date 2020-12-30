#include "PCH.h"
#include "MixTexture.h"
#include "../Common/Reflection/ReflectionClassDefine.hpp"
#include "../Common/Reflection/Types/ReflectionSharedPtrType.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::MixTexture)
{
    NFE_CLASS_PARENT(NFE::RT::ITexture);
    NFE_CLASS_MEMBER(mTextureA);
    NFE_CLASS_MEMBER(mTextureB);
    NFE_CLASS_MEMBER(mTextureMask);
}
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

MixTexture::MixTexture(const TexturePtr& textureA, const TexturePtr& textureB, const TexturePtr& textureMask)
    : mTextureA(textureA)
    , mTextureB(textureB)
    , mTextureMask(textureMask)
{
    NFE_ASSERT(mTextureA, "");
    NFE_ASSERT(mTextureB, "");
    NFE_ASSERT(mTextureMask, "");
}

const char* MixTexture::GetName() const
{
    return "mix";
}

const Vec4f MixTexture::Evaluate(const Vec4f& coords) const
{
    const Vec4f colorA = mTextureA->Evaluate(coords);
    const Vec4f colorB = mTextureB->Evaluate(coords);
    const Vec4f weight = mTextureMask->Evaluate(coords);

    return Vec4f::Lerp(colorA, colorB, weight);
}

const Vec4f MixTexture::Sample(const Vec2f u, Vec4f& outCoords, float* outPdf) const
{
    // TODO
    
    outCoords = Vec4f(u);

    if (outPdf)
    {
        *outPdf = 1.0f;
    }

    return MixTexture::Evaluate(outCoords);
}

} // namespace RT
} // namespace NFE
