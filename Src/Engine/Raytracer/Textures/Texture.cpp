#include "PCH.h"
#include "Texture.h"
#include "../Common/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::ITexture)
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

ITexture::~ITexture() = default;

bool ITexture::MakeSamplable(SampleDistortion distortion)
{
    NFE_UNUSED(distortion);
    return true;
}

bool ITexture::IsSamplable(SampleDistortion distortion) const
{
    NFE_UNUSED(distortion);
    return true;
}

float ITexture::Pdf(SampleDistortion distortion, const Math::Vec4f& coords) const
{
    NFE_UNUSED(distortion);
    NFE_UNUSED(coords);
    return 1.0f;
}

const Vec4f ITexture::Sample(const Vec3f u, Vec4f& outCoords, SampleDistortion distortion, float* outPdf) const
{
    NFE_UNUSED(u);
    NFE_UNUSED(outCoords);
    NFE_UNUSED(distortion);
    NFE_UNUSED(outPdf);

    NFE_FATAL("Not implemented");

    return Vec4f::Zero();
}

} // namespace RT
} // namespace NFE
