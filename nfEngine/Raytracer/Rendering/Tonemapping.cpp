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
    // based on https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    const Vector4 linearTonemapped = (hdrColor * (a * hdrColor + Vector4(b))) * Vector4::FastReciprocal(hdrColor * (c * hdrColor + Vector4(d)) + Vector4(e));

    return Convert_Linear_To_sRGB(linearTonemapped);
}

} // namespace RT
} // namespace NFE
