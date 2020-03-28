#include "PCH.h"
#include "PostProcess.h"
#include "Tonemapping.h"
#include "../Common/Math/Transcendental.hpp"
#include "../Common/Math/Vec4i.hpp"
#include "../Common/Math/Vec4fLoad.hpp"
#include "../Common/Containers/StaticArray.hpp"
#include "../Common/Reflection/ReflectionClassDefine.hpp"
#include "../Common/Reflection/Types/ReflectionUniquePtrType.hpp"
#include "../Common/Logger/Logger.hpp"

NFE_BEGIN_DEFINE_ENUM(NFE::RT::ColorSpace)
{
    NFE_ENUM_OPTION(Rec709);
    NFE_ENUM_OPTION(Rec2020);
}
NFE_END_DEFINE_ENUM()

NFE_DEFINE_CLASS(NFE::RT::BloomElement)
{
    NFE_CLASS_MEMBER(weight).Min(0.0f).Max(1.0f);
    NFE_CLASS_MEMBER(sigma).Min(0.0f).Max(100.0f);
    NFE_CLASS_MEMBER(numBlurPasses).Min(1).Max(64);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_CLASS(NFE::RT::BloomParams)
{
    NFE_CLASS_MEMBER(factor).Min(0.0f).Max(1.0f);
    NFE_CLASS_MEMBER(elements);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_CLASS(NFE::RT::ColorGradingParams)
{
    NFE_CLASS_MEMBER(gain);
    NFE_CLASS_MEMBER(contrast).Min(0.0f).Max(4.0f);
    NFE_CLASS_MEMBER(contrastMidPoint).Min(-10.0f).Max(10.0f);
    NFE_CLASS_MEMBER(saturation).Min(0.0f).Max(2.0f);
    NFE_CLASS_MEMBER(negative);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_CLASS(NFE::RT::PostprocessLutParams)
{
    NFE_CLASS_MEMBER(minValue).Min(1.0e-10f);// .Max(1.0e+10f).LogScale(10.0f);
    NFE_CLASS_MEMBER(maxValue).Max(1.0e+10f);// .LogScale(10.0f);
    NFE_CLASS_MEMBER(sizeShift).Min(1).Max(8);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_CLASS(NFE::RT::PostprocessParams)
{
    NFE_CLASS_MEMBER(exposure).Min(-10.0f).Max(10.0f);
    NFE_CLASS_MEMBER(filmGrainStrength).Min(0.0f).Max(1.0f);
    NFE_CLASS_MEMBER(colorGradingParams);
    NFE_CLASS_MEMBER(tonemapper).NonNull();
    NFE_CLASS_MEMBER(bloom);
    NFE_CLASS_MEMBER(useDithering);
    NFE_CLASS_MEMBER(lutParams);
    NFE_CLASS_MEMBER(colorSpace);
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

using namespace Math;
using namespace Common;

BloomParams::BloomParams()
    : factor(0.0f)
{
    elements.PushBack({ 0.35f,  2.0f, 6 });
    elements.PushBack({ 0.25f,  5.0f, 6 });
    elements.PushBack({ 0.15f, 12.0f, 6 });
    elements.PushBack({ 0.15f, 32.0f, 6 });
    elements.PushBack({ 0.1f,  80.0f, 6 });
}

ColorGradingParams::ColorGradingParams()
    : gain{ 1.0f, 1.0f, 1.0f }
    , contrast{ 1.0f, 1.0f, 1.0f }
    , contrastMidPoint(0.0f)
    , saturation(0.99f)
    , negative(false)
{
}

PostprocessParams::PostprocessParams()
    : exposure(0.0f)
    , filmGrainStrength(0.0f)
    , useDithering(true)
    , colorSpace(ColorSpace::Rec709)
    , tonemapper(Common::MakeUniquePtr<ApproxACESTonemapper>())
{
}

PostprocessParams::~PostprocessParams() = default;

const Vec4f PostprocessParams::Process(const Vec4f& inputColor) const
{
    Vec4f color = inputColor;

    // apply gain
    {
        color *= colorGradingParams.gain.ToVec4f();
    }

    // apply saturation
    {
        const float grayscale = Vec4f::Dot3(color, c_rgbIntensityWeights);
        color = Vec4f::Max(Vec4f::Zero(), Vec4f::Lerp(Vec4f(grayscale), color, colorGradingParams.saturation));
    }

    // apply contrast
    {
        const Vec4f logMidpoint(colorGradingParams.contrastMidPoint);
        const Vec4f logColor = FastLog(color);
        const Vec4f adjColor = logMidpoint + (logColor - logMidpoint) * colorGradingParams.contrast.ToVec4f();
        color = FastExp(adjColor);
    }

    // TODO more parameters:
    // - gamma
    // - lift
    // - shadows/mids/highlights
    // - hue

    // apply tonemapping
    color = tonemapper->Apply(color);

    if (colorGradingParams.negative)
    {
        color = Vec4f(1.0f) - color;
    }

    return color;
}

bool PostprocessLUT::Generate(const PostprocessParams& params)
{
    const uint32 lutSize = 1 << params.lutParams.sizeShift;

    if (lutSize < 2 || lutSize > MaxLutSize)
    {
        NFE_LOG_ERROR("Postprocess LUT size (%u) is out of range", lutSize);
        return false;
    }

    if (params.lutParams.minValue <= 0.0f || !IsValid(params.lutParams.minValue))
    {
        NFE_LOG_ERROR("Postprocess LUT min value is invalid (%f)", params.lutParams.minValue);
        return false;
    }

    if (params.lutParams.maxValue <= 0.0f || !IsValid(params.lutParams.maxValue))
    {
        NFE_LOG_ERROR("Postprocess LUT min value is invalid (%f)", params.lutParams.maxValue);
        return false;
    }

    if (params.lutParams.minValue >= params.lutParams.maxValue)
    {
        NFE_LOG_ERROR("Postprocess LUT min value must be smaller than max value");
        return false;
    }

    mSizeShift = params.lutParams.sizeShift;
    mMinValue = params.lutParams.minValue;
    mMaxValue = params.lutParams.maxValue;
    mScale = 1.0f / log2f(mMaxValue / mMinValue);
    mBias = -mScale * log2f(mMinValue);

    NFE_FREE(mLUT);
    mLUT = (Math::Half4*)NFE_MALLOC(lutSize * lutSize * lutSize * sizeof(Half4), sizeof(Half4));

    const float invSize = 1.0f / static_cast<float>(lutSize - 1u);

    StaticArray<float, MaxLutSize> precomputedColorValues;
    for (uint32 i = 0; i < lutSize; ++i)
    {
        const float colorValue = exp2f(((float)i * invSize - mBias) / mScale);
        precomputedColorValues.PushBack(colorValue);
    }

    uint32 index = 0;
    for (uint32 z = 0; z < lutSize; ++z)
    {
        const float b = precomputedColorValues[z];

        for (uint32 y = 0; y < lutSize; ++y)
        {
            const float g = precomputedColorValues[y];

            for (uint32 x = 0; x < lutSize; ++x)
            {
                const float r = precomputedColorValues[x];

                const Vec4f inputColor(r, g, b);
                const Vec4f outputColor = params.Process(inputColor);

                mLUT[index++] = outputColor.ToHalf4();
            }
        }
    }

    return true;
}

#pragma optimize("",off)

const Vec4f PostprocessLUT::Sample(const Vec4f& inputColor) const
{
    const uint32 lutSize = 1 << mSizeShift;

    const Vec4f color = Vec4f::Clamp(inputColor, Vec4f(mMinValue), Vec4f(mMaxValue));
    const Vec4f coord = mScale * FastLog2(color) + Vec4f(mBias);

    // compute texel coordinates
    const Vec4f scaledCoords = coord * static_cast<float>(lutSize - 1u);

    const Vec4i coordsA = Vec4i::TruncateAndConvert(scaledCoords);
    NFE_ASSERT(((coordsA >= Vec4i::Zero()) & (coordsA < Vec4i(lutSize))).All3());

    const Vec4i coordsB = Vec4i::Min(coordsA + Vec4i(1u), Vec4i(lutSize - 1));
    NFE_ASSERT(((coordsB >= Vec4i::Zero()) & (coordsB < Vec4i(lutSize))).All3());

    // trilinear interpolation

    const Half4* rowData0 = mLUT + ((coordsA.y + (coordsA.z << mSizeShift)) << mSizeShift);
    const Half4* rowData1 = mLUT + ((coordsB.y + (coordsA.z << mSizeShift)) << mSizeShift);
    const Half4* rowData2 = mLUT + ((coordsA.y + (coordsB.z << mSizeShift)) << mSizeShift);
    const Half4* rowData3 = mLUT + ((coordsB.y + (coordsB.z << mSizeShift)) << mSizeShift);

    Vec4f colors[8] =
    {
        Vec4f::FromHalf4(rowData0[coordsA.x]),
        Vec4f::FromHalf4(rowData0[coordsB.x]),
        Vec4f::FromHalf4(rowData1[coordsA.x]),
        Vec4f::FromHalf4(rowData1[coordsB.x]),
        Vec4f::FromHalf4(rowData2[coordsA.x]),
        Vec4f::FromHalf4(rowData2[coordsB.x]),
        Vec4f::FromHalf4(rowData3[coordsA.x]),
        Vec4f::FromHalf4(rowData3[coordsB.x]),
    };

    const Vec4f weights = scaledCoords - coordsA.ConvertToVec4f();

    const Vec4f value00 = Vec4f::Lerp(colors[0], colors[1], weights.SplatX());
    const Vec4f value01 = Vec4f::Lerp(colors[2], colors[3], weights.SplatX());
    const Vec4f value10 = Vec4f::Lerp(colors[4], colors[5], weights.SplatX());
    const Vec4f value11 = Vec4f::Lerp(colors[6], colors[7], weights.SplatX());

    const Vec4f value0 = Vec4f::Lerp(value00, value01, weights.SplatY());
    const Vec4f value1 = Vec4f::Lerp(value10, value11, weights.SplatY());

    return Vec4f::Lerp(value0, value1, weights.SplatZ());
}

} // namespace RT
} // namespace NFE
