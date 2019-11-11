#pragma once

#include "../Raytracer.h"
#include "../../nfCommon/Math/Vector4.hpp"


namespace NFE {
namespace RT {

NFE_GLOBAL_CONST Math::Vector4 c_rgbIntensityWeights = { 0.2126f, 0.7152f, 0.0722f, 0.0f };

NFE_GLOBAL_CONST Math::Vector4 XYZtoRGB_r = {  3.240479f, -1.537150f, -0.498535f, 0.0f };
NFE_GLOBAL_CONST Math::Vector4 XYZtoRGB_g = { -0.969256f,  1.875991f,  0.041556f, 0.0f };
NFE_GLOBAL_CONST Math::Vector4 XYZtoRGB_b = {  0.055648f, -0.204043f,  1.057311f, 0.0f };

// Convert linear to sRGB
template<typename T>
NFE_FORCE_INLINE Math::Vector4 Convert_sRGB_To_Linear(const T& gammaColor)
{
    // based on:
    // http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html

    T result;
    result = T::MulAndAdd(gammaColor, T(0.305306011f), T(0.682171111f));
    result = T::MulAndAdd(gammaColor, result, T(0.012522878f));
    result *= gammaColor;
    return result;
}

// Convert sRGB to linear
template<typename T>
NFE_FORCE_INLINE Math::Vector4 Convert_Linear_To_sRGB(const T& linearColor)
{
    // based on:
    // http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html

    const T s1 = T::Sqrt(linearColor);
    const T s2 = T::Sqrt(s1);
    const T s3 = T::Sqrt(s2);

    T result = 0.585122381f * s1;
    result = T::MulAndAdd(s2, 0.783140355f, result);
    result = T::MulAndAdd(s3, -0.368262736f, result);
    return T::Saturate(result);
}

// Convert CIE XYZ to linear RGB (Rec. BT.709)
NFE_FORCE_INLINE Math::Vector4 ConvertXYZtoRGB(const Math::Vector4& xyzColor)
{
    Math::Vector4 r = XYZtoRGB_r * xyzColor;
    Math::Vector4 g = XYZtoRGB_g * xyzColor;
    Math::Vector4 b = XYZtoRGB_b * xyzColor;

    Math::Vector4::Transpose3(r, g, b);

    return r + g + b;
}

// Convert linear RGB (Rec. BT.709) to CIE XYZ
NFE_FORCE_INLINE Math::Vector4 ConvertRGBtoXYZ(const Math::Vector4& rgbColor)
{
    const float mapping[3][3] =
    {
        { 0.412453f, 0.357580f, 0.180423f },
        { 0.212671f, 0.715160f, 0.072169f },
        { 0.019334f, 0.119193f, 0.950227f }
    };

    return Math::Vector4
    (
        mapping[0][0] * rgbColor[0] + mapping[0][1] * rgbColor[1] + mapping[0][2] * rgbColor[2],
        mapping[1][0] * rgbColor[0] + mapping[1][1] * rgbColor[1] + mapping[1][2] * rgbColor[2],
        mapping[2][0] * rgbColor[0] + mapping[2][1] * rgbColor[1] + mapping[2][2] * rgbColor[2],
        0.0f
    );
}

// Convert HSV to linear RGB
NFE_FORCE_INLINE Math::Vector4 HSVtoRGB(const float hue, const float saturation, const float value)
{
    const int h_i = (int)(hue * 6.0f);
    const float f = hue * 6 - h_i;
    const float p = value * (1 - saturation);
    const float q = value * (1 - f * saturation);
    const float t = value * (1 - (1 - f) * saturation);

    if (h_i == 0)
        return Math::Vector4(value, t, p, 0.0f);
    else if (h_i == 1)
        return Math::Vector4(q, value, p, 0.0f);
    else if (h_i == 2)
        return Math::Vector4(p, value, t, 0.0f);
    else if (h_i == 3)
        return Math::Vector4(p, q, value, 0.0f);
    else if (h_i == 4)
        return Math::Vector4(t, p, value, 0.0f);
    else if (h_i == 5)
        return Math::Vector4(value, p, q, 0.0f);

    return Math::Vector4::Zero();
}


} // namespace RT
} // namespace NFE
