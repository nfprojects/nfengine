#include "PCH.h"
#include "CheckerboardTexture.h"
#include "../../Common/Math/ColorHelpers.hpp"

namespace NFE {
namespace RT {

using namespace Math;

CheckerboardTexture::CheckerboardTexture(const Math::Vec4f& colorA, const Math::Vec4f& colorB)
    : mColorA(colorA)
    , mColorB(colorB)
    , mPdf(0.5f)
{
    NFE_ASSERT(colorA.IsValid(), "");
    NFE_ASSERT(colorB.IsValid(), "");

    // compute probability of sampling color A
    const float colorWeightA = Vec4f::Dot3(colorA, c_rgbIntensityWeights);
    const float colorWeightB = Vec4f::Dot3(colorB, c_rgbIntensityWeights);
    if (colorWeightA > FLT_EPSILON || colorWeightB > FLT_EPSILON)
    {
        mPdf = colorWeightA / (colorWeightA + colorWeightB);
    }
}

const char* CheckerboardTexture::GetName() const
{
    return "checkerboard";
}

const Vec4f CheckerboardTexture::Evaluate(const Vec4f& coords) const
{
    // wrap to 0..1 range
    const Vec4f warpedCoords = Vec4f::Mod1(coords);

    const VecBool4f conditionVec = warpedCoords > VECTOR_HALVES;

    return (conditionVec.Get<0>() ^ conditionVec.Get<1>() ^ conditionVec.Get<2>()) ? mColorA : mColorB;
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
