#include "PCH.h"
#include "Texture.h"
#include "../Common/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::ITexture)
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

ITexture::~ITexture() = default;

bool ITexture::MakeSamplable()
{
    return true;
}

bool ITexture::IsSamplable() const
{
    return true;
}

const Vec4f ITexture::Sample(const Vec2f u, Vec4f& outCoords, float* outPdf) const
{
    NFE_UNUSED(u);
    NFE_UNUSED(outCoords);
    NFE_UNUSED(outPdf);

    NFE_FATAL("Not implemented");

    return Vec4f::Zero();
}

} // namespace RT
} // namespace NFE
