#include "PCH.h"
#include "Tonemapping.h"
#include "../nfCommon/Reflection/ReflectionClassDefine.hpp"
#include "../nfCommon/Math/Vector4.hpp"
#include "../nfCommon/Math/ColorHelpers.hpp"
#include "../nfCommon/Math/Transcendental.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::ITonemapper)
NFE_END_DEFINE_CLASS()

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::DebugTonemapper)
{
    NFE_CLASS_PARENT(NFE::RT::ITonemapper);
    NFE_CLASS_MEMBER(mMinValue).Min(1.0e-10f).LogScale(2.0f);
    NFE_CLASS_MEMBER(mMaxValue).Max(1.0e+10f).LogScale(2.0f);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::ClampedTonemapper)
{
    NFE_CLASS_PARENT(NFE::RT::ITonemapper);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::ReinhardTonemapper)
{
    NFE_CLASS_PARENT(NFE::RT::ITonemapper);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::FilmicTonemapper)
{
    NFE_CLASS_PARENT(NFE::RT::ITonemapper);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::ApproxACESTonemapper)
{
    NFE_CLASS_PARENT(NFE::RT::ITonemapper);
    NFE_CLASS_MEMBER(fitted);
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

using namespace Math;

DebugTonemapper::DebugTonemapper()
    : mMinValue(1.0e-3f)
    , mMaxValue(1.0e+3f)
{}

const Vector4 DebugTonemapper::Apply(const Vector4 hdrColor) const
{
    const Vector4 min(mMinValue);
    const Vector4 max(mMaxValue);

    return FastLog(Vector4::Clamp(hdrColor, min, max) / min) / FastLog(max / min);
}

const Vector4 ClampedTonemapper::Apply(const Vector4 hdrColor) const
{
    return Convert_Linear_To_sRGB(hdrColor);
}

const Vector4 ReinhardTonemapper::Apply(const Vector4 hdrColor) const
{
    return Convert_Linear_To_sRGB(hdrColor / (hdrColor + Vector4(1.0f)));
}

const Vector4 FilmicTonemapper::Apply(const Vector4 hdrColor) const
{
    const Vector4 b(6.2f);
    const Vector4 c(1.7f);
    const Vector4 d(0.06f);
    const Vector4 t0 = hdrColor * Vector4::MulAndAdd(hdrColor, b, Vector4(0.5f));
    const Vector4 t1 = Vector4::MulAndAdd(hdrColor, b, c);
    const Vector4 t2 = Vector4::MulAndAdd(hdrColor, t1, d);
    return t0 * Vector4::FastReciprocal(t2);
}

const Vector4 ApproxACESTonemapper::Apply(const Vector4 hdrColor) const
{
    Vector4 color;

    if (fitted)
    {
        // based on https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl

        // sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
        const Vector4 ACESInputMat[] =
        {
            { 0.59719f, 0.07600f, 0.02840f, 0.0f },
            { 0.35458f, 0.90834f, 0.13383f, 0.0f },
            { 0.04823f, 0.01566f, 0.83777f, 0.0f },
        };

        // ODT_SAT => XYZ => D60_2_D65 => sRGB
        const Vector4 ACESOutputMat[] =
        {
            {  1.60475f, -0.10208f, -0.00327f, 0.0f },
            { -0.53108f,  1.10813f, -0.07276f, 0.0f },
            { -0.07367f, -0.00605f,  1.07602f, 0.0f },
        };

        const Vector4 v = ACESInputMat[0] * hdrColor.x + ACESInputMat[1] * hdrColor.y + ACESInputMat[2] * hdrColor.z;

        const Vector4 a = v * (v + Vector4(0.0245786f)) - Vector4(0.000090537f);
        const Vector4 b = v * (0.983729f * v + Vector4(0.4329510f)) + Vector4(0.238081f);
        color = a * Vector4::FastReciprocal(b);

        color = Vector4::Saturate(ACESOutputMat[0] * color.x + ACESOutputMat[1] * color.y + ACESOutputMat[2] * color.z);
    }
    else
    {
        // based on https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/

        color = 0.5f * hdrColor;

        const Vector4 a = color * (2.51f * color + Vector4(0.03f));
        const Vector4 b = color * (2.43f * color + Vector4(0.59f)) + Vector4(0.14f);

        color = a / b;
    }

    return Convert_Linear_To_sRGB(color);

}

} // namespace RT
} // namespace NFE
