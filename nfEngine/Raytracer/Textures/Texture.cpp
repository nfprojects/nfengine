#include "PCH.h"
#include "Texture.h"

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

const Vector4 ITexture::Sample(const Float2 u, Vector4& outCoords, float* outPdf) const
{
    NFE_UNUSED(u);
    NFE_UNUSED(outCoords);
    NFE_UNUSED(outPdf);

    NFE_FATAL("Not implemented");

    return Vector4::Zero();
}

} // namespace RT
} // namespace NFE
