#pragma once

#include "Vec4f.hpp"


namespace NFE {
namespace Math {

NFE_GLOBAL_CONST Vec4f c_rgbIntensityWeights = { 0.2126f, 0.7152f, 0.0722f, 0.0f };

NFE_GLOBAL_CONST Vec4f XYZtoRGB_r = {  3.240479f, -1.537150f, -0.498535f, 0.0f };
NFE_GLOBAL_CONST Vec4f XYZtoRGB_g = { -0.969256f,  1.875991f,  0.041556f, 0.0f };
NFE_GLOBAL_CONST Vec4f XYZtoRGB_b = {  0.055648f, -0.204043f,  1.057311f, 0.0f };

// Convert linear to sRGB
template<typename T>
NFE_FORCE_INLINE const T Convert_sRGB_To_Linear(const T& gammaColor)
{
    // based on:
    // http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html

    T result;
    result = gammaColor * T(0.305306011f) + T(0.682171111f);
    result = gammaColor * result + T(0.012522878f);
    result *= gammaColor;
    return result;
}

// Convert sRGB to linear
template<typename T>
NFE_FORCE_INLINE const T Convert_Linear_To_sRGB(const T& linearColor)
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
NFE_FORCE_INLINE Vec4f ConvertXYZtoRec709(const Vec4f& xyzColor)
{
    Vec4f r = Vec4f( 3.2409699419f, -1.5373831776f, -0.4986107603f) * xyzColor;
    Vec4f g = Vec4f(-0.9692436363f,  1.8759675015f,  0.0415550574f) * xyzColor;
    Vec4f b = Vec4f( 0.0556300797f, -0.2039769589f,  1.0569715142f) * xyzColor;

    Vec4f::Transpose3(r, g, b);

    return r + g + b;
}

// Convert CIE XYZ to linear RGB (Rec. BT.2020)
NFE_FORCE_INLINE Vec4f ConvertXYZtoRec2020(const Vec4f& xyzColor)
{
    Vec4f r = Vec4f( 1.7166511880f, -0.3556707838f, -0.2533662814f) * xyzColor;
    Vec4f g = Vec4f(-0.6666843518f,  1.6164812366f,  0.0157685458f) * xyzColor;
    Vec4f b = Vec4f( 0.0176398574f, -0.0427706133f,  0.9421031212f) * xyzColor;

    Vec4f::Transpose3(r, g, b);

    return r + g + b;
}

// Convert linear RGB (Rec. BT.709) to CIE XYZ
NFE_FORCE_INLINE Vec4f ConvertRec709toXYZ(const Vec4f& rgbColor)
{
    const float mapping[3][3] =
    {
        { 0.412453f, 0.357580f, 0.180423f },
        { 0.212671f, 0.715160f, 0.072169f },
        { 0.019334f, 0.119193f, 0.950227f }
    };

    return Vec4f
    (
        mapping[0][0] * rgbColor[0] + mapping[0][1] * rgbColor[1] + mapping[0][2] * rgbColor[2],
        mapping[1][0] * rgbColor[0] + mapping[1][1] * rgbColor[1] + mapping[1][2] * rgbColor[2],
        mapping[2][0] * rgbColor[0] + mapping[2][1] * rgbColor[1] + mapping[2][2] * rgbColor[2],
        0.0f
    );
}

// Convert HSV to linear RGB
NFE_FORCE_INLINE Vec4f HSVtoRGB(const float hue, const float saturation, const float value)
{
    const int h_i = (int)(hue * 6.0f);
    const float f = hue * 6 - h_i;
    const float p = value * (1 - saturation);
    const float q = value * (1 - f * saturation);
    const float t = value * (1 - (1 - f) * saturation);

    if (h_i == 0)
        return Vec4f(value, t, p, 0.0f);
    else if (h_i == 1)
        return Vec4f(q, value, p, 0.0f);
    else if (h_i == 2)
        return Vec4f(p, value, t, 0.0f);
    else if (h_i == 3)
        return Vec4f(p, q, value, 0.0f);
    else if (h_i == 4)
        return Vec4f(t, p, value, 0.0f);
    else if (h_i == 5)
        return Vec4f(value, p, q, 0.0f);

    return Vec4f::Zero();
}


} // namespace Math
} // namespace NFE
