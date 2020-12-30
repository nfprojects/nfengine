#include "PCH.h"
#include "ConstTexture.h"
#include "../Common/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::ConstTexture)
{
    NFE_CLASS_PARENT(NFE::RT::ITexture);
    NFE_CLASS_MEMBER(mColor);
}
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

ConstTexture::ConstTexture(const Math::Vec4f& color)
    : mColor(color)
{
    NFE_ASSERT(mColor.IsValid(), "");
}

const char* ConstTexture::GetName() const
{
    return "const";
}

const Vec4f ConstTexture::Evaluate(const Vec4f& coords) const
{
    NFE_UNUSED(coords);

    return mColor.ToVec4f();
}

const Vec4f ConstTexture::Sample(const Vec2f u, Vec4f& outCoords, float* outPdf) const
{
    outCoords = Vec4f(u);

    if (outPdf)
    {
        *outPdf = 1.0f;
    }

    return mColor.ToVec4f();
}

} // namespace RT
} // namespace NFE
