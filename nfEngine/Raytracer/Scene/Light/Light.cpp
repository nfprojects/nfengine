#include "PCH.h"
#include "Light.h"
#include "../nfCommon/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::ILight)
{
    NFE_CLASS_MEMBER(mColor);
}
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

ILight::ILight(const Math::HdrColorRGB& color)
{
    // TODO generic spectrum
    NFE_ASSERT(color.IsValid());
    mColor = color;
}

void ILight::SetColor(const Math::HdrColorRGB& color)
{
    NFE_ASSERT(color.IsValid());
    mColor = color;
}

const RayColor ILight::GetRadiance(const RadianceParam&, float*, float*) const
{
    NFE_FATAL("Cannot hit this type of light");
    return RayColor();
}

} // namespace RT
} // namespace NFE
