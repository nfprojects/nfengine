#include "PCH.h"
#include "PostProcess.h"
#include "Tonemapping.h"
#include "../nfCommon/Reflection/ReflectionClassDefine.hpp"
#include "../nfCommon/Reflection/Types/ReflectionUniquePtrType.hpp"

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

NFE_DEFINE_CLASS(NFE::RT::PostprocessParams)
{
    NFE_CLASS_MEMBER(colorFilter);
    NFE_CLASS_MEMBER(exposure).Min(-10.0f).Max(10.0f);
    NFE_CLASS_MEMBER(contrast).Min(0.0f).Max(2.0f);
    NFE_CLASS_MEMBER(saturation).Min(0.0f).Max(2.0f);
    NFE_CLASS_MEMBER(useDithering);
    NFE_CLASS_MEMBER(bloom);
    NFE_CLASS_MEMBER(tonemapper).NonNull();
    NFE_CLASS_MEMBER(colorSpace);
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

BloomParams::BloomParams()
    : factor(0.0f)
{
    elements.PushBack({ 0.35f,  2.0f, 6 });
    elements.PushBack({ 0.25f,  5.0f, 6 });
    elements.PushBack({ 0.15f, 12.0f, 6 });
    elements.PushBack({ 0.15f, 32.0f, 6 });
    elements.PushBack({ 0.1f,  80.0f, 6 });
}

PostprocessParams::PostprocessParams()
    : colorFilter(1.0f, 1.0f, 1.0f)
    , exposure(0.0f)
    , contrast(0.8f)
    , saturation(0.98f)
    , useDithering(true)
    , colorSpace(ColorSpace::Rec709)
    , tonemapper(Common::MakeUniquePtr<ApproxACESTonemapper>())
{
}

PostprocessParams::~PostprocessParams() = default;

} // namespace RT
} // namespace NFE
