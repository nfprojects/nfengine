#include "PCH.h"
#include "ConstTexture.h"

namespace NFE {
namespace RT {

using namespace Math;

ConstTexture::ConstTexture(const Math::Vec4f& color)
    : mColor(color)
{
    NFE_ASSERT(mColor.IsValid());
}

const char* ConstTexture::GetName() const
{
    return "const";
}

const Vec4f ConstTexture::Evaluate(const Vec4f& coords) const
{
    NFE_UNUSED(coords);

    return mColor;
}

const Vec4f ConstTexture::Sample(const Vec2f u, Vec4f& outCoords, float* outPdf) const
{
    outCoords = Vec4f(u);

    if (outPdf)
    {
        *outPdf = 1.0f;
    }

    return mColor;
}

} // namespace RT
} // namespace NFE
