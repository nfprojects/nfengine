#include "PCH.h"
#include "CheckerboardTexture.h"
#include "../../Common/Math/ColorHelpers.hpp"
#include "../../Common/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::CheckerboardTexture)
{
    NFE_CLASS_PARENT(NFE::RT::ITexture);
    NFE_CLASS_MEMBER(mColorA);
    NFE_CLASS_MEMBER(mColorB);
}
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

CheckerboardTexture::CheckerboardTexture()
    : mColorA(Vec4f(1.0f))
    , mColorB(Vec4f(0.0f))
    , mPdf(0.5f)
{}

CheckerboardTexture::CheckerboardTexture(const Vec4f& colorA, const Vec4f& colorB)
    : mColorA(colorA)
    , mColorB(colorB)
    , mPdf(0.5f)
{
    NFE_ASSERT(colorA.IsValid(), "");
    NFE_ASSERT(colorB.IsValid(), "");

    UpdatePdf();
}

bool CheckerboardTexture::OnPropertyChanged(const Common::StringView propertyName)
{
    if (propertyName == "mColorA" || propertyName == "mColorB")
    {
        UpdatePdf();
        return true;
    }

    return ITexture::OnPropertyChanged(propertyName);
}

const char* CheckerboardTexture::GetName() const
{
    return "checkerboard";
}

void CheckerboardTexture::UpdatePdf()
{
    // compute probability of sampling color A
    const float colorWeightA = mColorA.Luminance();
    const float colorWeightB = mColorB.Luminance();
    if (colorWeightA > FLT_EPSILON || colorWeightB > FLT_EPSILON)
    {
        mPdf = colorWeightA / (colorWeightA + colorWeightB);
    }
    else
    {
        mPdf = 0.5f;
    }
}

const Vec4f CheckerboardTexture::Evaluate(const Vec4f& coords) const
{
    // wrap to 0..1 range
    const Vec4f warpedCoords = Vec4f::Mod1(coords);

    const VecBool4f conditionVec = warpedCoords > VECTOR_HALVES;

    return (conditionVec.Get<0>() ^ conditionVec.Get<1>() ^ conditionVec.Get<2>()) ? mColorA.ToVec4f() : mColorB.ToVec4f();
}

const Vec4f CheckerboardTexture::Sample(const Vec2f u, Vec4f& outCoords, float* outPdf) const
{
    // TODO

    outCoords = Vec4f(u);

    if (outPdf)
    {
        *outPdf = 1.0f;
    }

    return CheckerboardTexture::Evaluate(outCoords);
}

} // namespace RT
} // namespace NFE
