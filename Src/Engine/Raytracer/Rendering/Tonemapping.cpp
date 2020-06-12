#include "PCH.h"
#include "Tonemapping.h"
#include "../Common/Reflection/ReflectionClassDefine.hpp"
#include "../Common/Math/ColorHelpers.hpp"
#include "../Common/Math/Transcendental.hpp"


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
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

using namespace Math;

DebugTonemapper::DebugTonemapper()
    : mMinValue(1.0e-3f)
    , mMaxValue(1.0e+3f)
{}

const Vec4f DebugTonemapper::Apply(const Vec4f hdrColor) const
{
    const Vec4f min(mMinValue);
    const Vec4f max(mMaxValue);

    return FastLog(Vec4f::Clamp(hdrColor, min, max) / min) / FastLog(max / min);
}

const Vec4f ClampedTonemapper::Apply(const Vec4f hdrColor) const
{
    return Convert_Linear_To_sRGB(hdrColor);
}

const Vec4f ReinhardTonemapper::Apply(const Vec4f hdrColor) const
{
    return Convert_Linear_To_sRGB(hdrColor / (hdrColor + Vec4f(1.0f)));
}

const Vec4f FilmicTonemapper::Apply(const Vec4f hdrColor) const
{
    const Vec4f b(6.2f);
    const Vec4f c(1.7f);
    const Vec4f d(0.06f);
    const Vec4f t0 = hdrColor * Vec4f::MulAndAdd(hdrColor, b, Vec4f(0.5f));
    const Vec4f t1 = Vec4f::MulAndAdd(hdrColor, b, c);
    const Vec4f t2 = Vec4f::MulAndAdd(hdrColor, t1, d);
    return t0 * Vec4f::FastReciprocal(t2);
}

const Vec4f ApproxACESTonemapper::Apply(const Vec4f hdrColor) const
{
    // based on https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl

    // sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
    const Vec4f ACESInputMat[] =
    {
        { 0.59719f, 0.07600f, 0.02840f, 0.0f },
        { 0.35458f, 0.90834f, 0.13383f, 0.0f },
        { 0.04823f, 0.01566f, 0.83777f, 0.0f },
    };

    // ODT_SAT => XYZ => D60_2_D65 => sRGB
    const Vec4f ACESOutputMat[] =
    {
        {  1.60475f, -0.10208f, -0.00327f, 0.0f },
        { -0.53108f,  1.10813f, -0.07276f, 0.0f },
        { -0.07367f, -0.00605f,  1.07602f, 0.0f },
    };

    const Vec4f v = ACESInputMat[0] * hdrColor.x + ACESInputMat[1] * hdrColor.y + ACESInputMat[2] * hdrColor.z;

    const Vec4f a = v * (v + Vec4f(0.0245786f)) - Vec4f(0.000090537f);
    const Vec4f b = v * (0.983729f * v + Vec4f(0.4329510f)) + Vec4f(0.238081f);
    Vec4f color = a * Vec4f::FastReciprocal(b);

    color = Vec4f::Saturate(ACESOutputMat[0] * color.x + ACESOutputMat[1] * color.y + ACESOutputMat[2] * color.z);

    return Convert_Linear_To_sRGB(color);

}

} // namespace RT
} // namespace NFE
