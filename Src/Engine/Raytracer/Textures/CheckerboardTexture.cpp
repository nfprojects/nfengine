#include "PCH.h"
#include "CheckerboardTexture.h"
#include "../../Common/Math/ColorHelpers.hpp"

namespace NFE {
namespace RT {

using namespace Math;

CheckerboardTexture::CheckerboardTexture(const Math::Vector4& colorA, const Math::Vector4& colorB)
    : mColorA(colorA)
    , mColorB(colorB)
    , mPdf(0.5f)
{
    NFE_ASSERT(colorA.IsValid());
    NFE_ASSERT(colorB.IsValid());

    // compute probability of sampling color A
    const float colorWeightA = Vector4::Dot3(colorA, c_rgbIntensityWeights);
    const float colorWeightB = Vector4::Dot3(colorB, c_rgbIntensityWeights);
    if (colorWeightA > FLT_EPSILON || colorWeightB > FLT_EPSILON)
    {
        mPdf = colorWeightA / (colorWeightA + colorWeightB);
    }
}

const char* CheckerboardTexture::GetName() const
{
    return "checkerboard";
}

const Vector4 CheckerboardTexture::Evaluate(const Vector4& coords) const
{
    // wrap to 0..1 range
    const Vector4 warpedCoords = Vector4::Mod1(coords);

    const VectorBool4 conditionVec = warpedCoords > VECTOR_HALVES;

    return (conditionVec.Get<0>() ^ conditionVec.Get<1>() ^ conditionVec.Get<2>()) ? mColorA : mColorB;
}

const Vector4 CheckerboardTexture::Sample(const Float2 u, Vector4& outCoords, float* outPdf) const
{
    // TODO

    outCoords = Vector4(u);

    if (outPdf)
    {
        *outPdf = 1.0f;
    }

    return CheckerboardTexture::Evaluate(outCoords);
}

} // namespace RT
} // namespace NFE
