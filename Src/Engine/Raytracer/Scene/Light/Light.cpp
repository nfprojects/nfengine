#include "PCH.h"
#include "Light.h"
#include "../Common/Reflection/ReflectionClassDefine.hpp"
#include "../Common/Reflection/Types/ReflectionSharedPtrType.hpp"

#include "../../Color/ColorRGB.h"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::ILight)
{
    NFE_CLASS_MEMBER(mColor).NonNull();
}
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

ILight::ILight(const Math::HdrColorRGB& color)
{
    NFE_ASSERT(color.IsValid(), "");
    mColor = Common::MakeSharedPtr<ColorRGB>(color);
}

void ILight::SetColor(const ColorPtr& color)
{
    NFE_ASSERT(color, "Invalid color");
    mColor = color;
}

const RayColor ILight::GetRadiance(const RadianceParam&, float*, float*) const
{
    NFE_FATAL("Cannot hit this type of light");
    return RayColor();
}

void ILight::Traverse(const SingleTraversalContext&, const uint32) const
{
    NFE_FATAL("Cannot hit this type of light");
}

bool ILight::Traverse_Shadow(const SingleTraversalContext&) const
{
    NFE_FATAL("Cannot hit this type of light");
    return false;
}

} // namespace RT
} // namespace NFE
