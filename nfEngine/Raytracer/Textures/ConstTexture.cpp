#include "PCH.h"
#include "ConstTexture.h"

namespace NFE {
namespace RT {

using namespace Math;

ConstTexture::ConstTexture(const Math::Vector4& color)
    : mColor(color)
{
    NFE_ASSERT(mColor.IsValid());
}

const char* ConstTexture::GetName() const
{
    return "const";
}

const Vector4 ConstTexture::Evaluate(const Vector4& coords) const
{
    NFE_UNUSED(coords);

    return mColor;
}

const Vector4 ConstTexture::Sample(const Float2 u, Vector4& outCoords, float* outPdf) const
{
    outCoords = Vector4(u);

    if (outPdf)
    {
        *outPdf = 1.0f;
    }

    return mColor;
}

} // namespace RT
} // namespace NFE
