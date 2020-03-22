#pragma once

#include "../Raytracer.h"
#include "../../Common/Math/HdrColor.hpp"
#include "../../Common/Reflection/ReflectionClassDeclare.hpp"
#include "../../Common/Reflection/ReflectionEnumMacros.hpp"
#include "../../Common/Reflection/Types/ReflectionDynArrayType.hpp"

namespace NFE {
namespace RT {

class ITonemapper;
using TonemapperPtr = Common::UniquePtr<ITonemapper>;

enum class ColorSpace : uint8
{
    Rec709,
    Rec2020,
};

class NFE_ALIGN(16) BloomElement
{
    NFE_DECLARE_CLASS(BloomElement);
public:

    float weight = 0.0f;
    float sigma = 5.0f;
    uint32 numBlurPasses = 8;
};

class NFE_ALIGN(16) BloomParams
{
    NFE_DECLARE_CLASS(BloomParams);
public:

    float factor; // bloom multiplier
    Common::DynArray<BloomElement> elements;

    NFE_RAYTRACER_API BloomParams();
};

class PostprocessLutParams
{
    NFE_DECLARE_CLASS(PostprocessLutParams);
public:
    uint32 sizeShift = 6;
    float minValue = 1.0e-4f;
    float maxValue = 1.0e+3f;
};

class ColorGradingParams
{
    NFE_DECLARE_CLASS(ColorGradingParams);
public:
    Math::HdrColorRGB gain;
    Math::HdrColorRGB contrast;
    float contrastMidPoint;     // midpoint for contrast in log scale
    float saturation;
    bool negative;

    ColorGradingParams();
};

class NFE_ALIGN(16) PostprocessParams
{
    NFE_DECLARE_CLASS(PostprocessParams);

public:

    // exposure in log scale
    float exposure;

    // pre-tonemapping film grain
    float filmGrainStrength;

    ColorGradingParams colorGradingParams;
    TonemapperPtr tonemapper;
    PostprocessLutParams lutParams;

    BloomParams bloom;

    bool useDithering;
    ColorSpace colorSpace;

    NFE_RAYTRACER_API PostprocessParams();
    NFE_RAYTRACER_API ~PostprocessParams();

    const Math::Vector4 Process(const Math::Vector4& inputColor) const;
};

class PostprocessLUT
{
public:
    static constexpr uint32 MaxLutSize = 256;

    // generate LUT given postprocessing parameters
    bool Generate(const PostprocessParams& postprocessParams);

    // sample LUT
    const Math::Vector4 Sample(const Math::Vector4& inputColor) const;

    NFE_FORCE_INLINE bool IsGenerated() const { return mLUT != nullptr; }

private:
    Math::Half4* mLUT = nullptr;
    uint32 mSizeShift;
    float mScale;
    float mBias;
    float mMinValue;
    float mMaxValue;
};

} // namespace RT
} // namespace NFE

NFE_DECLARE_ENUM_TYPE(NFE::RT::ColorSpace);
