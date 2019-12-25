#include "PCH.h"
#include "ColorRGB.h"
#include "../../nfCommon/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::ColorRGB)
{
    NFE_CLASS_PARENT(NFE::RT::IColor);
    NFE_CLASS_MEMBER(mColor);
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

using namespace Math;

bool ColorRGB::IsValid() const
{
    return mColor.IsValid();
}

const RayColor ColorRGB::Resolve(const Wavelength& wavelength) const
{
    return RayColor::ResolveRGB(wavelength, mColor);
}

} // namespace RT
} // namespace NFE
