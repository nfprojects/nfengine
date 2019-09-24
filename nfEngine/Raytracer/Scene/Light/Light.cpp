#include "PCH.h"
#include "Light.h"

namespace NFE {
namespace RT {

using namespace Math;

ILight::ILight(const Vector4& color)
{
    // TODO generic spectrum
    NFE_ASSERT(color.IsValid());
    NFE_ASSERT((color >= Vector4::Zero()).All());
    mColor.rgbValues = color;
}

void ILight::SetColor(const Spectrum& color)
{
    NFE_ASSERT((color.rgbValues >= Vector4::Zero()).All(), "Invalid color");
    NFE_ASSERT(color.rgbValues.IsValid(), "Invalid color");
    mColor = color;
}

const RayColor ILight::GetRadiance(const RadianceParam&, float*, float*) const
{
    NFE_FATAL("Cannot hit this type of light");
    return RayColor();
}

} // namespace RT
} // namespace NFE
