#include "PCH.h"
#include "PostProcess.h"

NFE_BEGIN_DEFINE_ENUM(NFE::RT::Tonemapper)
{
    NFE_ENUM_OPTION(Clamped);
    NFE_ENUM_OPTION(Reinhard);
    NFE_ENUM_OPTION(JimHejland_RichardBurgessDawson);
    NFE_ENUM_OPTION(ACES);
}
NFE_END_DEFINE_ENUM()

NFE_BEGIN_DEFINE_CLASS(NFE::RT::PostprocessParams)
{
    NFE_CLASS_MEMBER(colorFilter);
    NFE_CLASS_MEMBER(exposure).Min(-10.0f).Max(10.0f);
    NFE_CLASS_MEMBER(contrast).Min(0.0f).Max(2.0f);
    NFE_CLASS_MEMBER(saturation).Min(0.0f).Max(2.0f);
    NFE_CLASS_MEMBER(ditheringStrength).Min(0.0f).Max(1.0f);
    NFE_CLASS_MEMBER(bloomFactor).Min(0.0f).Max(1.0f);
    NFE_CLASS_MEMBER(tonemapper);
}
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

PostprocessParams::PostprocessParams()
    : colorFilter(1.0f, 1.0f, 1.0f)
    , exposure(0.0f)
    , contrast(0.8f)
    , saturation(0.98f)
    , ditheringStrength(0.005f)
    , bloomFactor(0.0f)
    , tonemapper(Tonemapper::ACES)
{
}

} // namespace RT
} // namespace NFE
