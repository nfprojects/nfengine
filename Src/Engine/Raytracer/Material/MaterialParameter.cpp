#include "PCH.h"
#include "MaterialParameter.h"
#include "../Color/ColorRGB.h"
#include "../Common/Reflection/ReflectionClassDefine.hpp"
#include "../Common/Reflection/Types/ReflectionSharedPtrType.hpp"


NFE_DEFINE_CLASS(NFE::RT::MaterialParameter)
{
    NFE_CLASS_MEMBER(baseValue).Min(0.0f).Max(1.0f);
    // TODO texture
}
NFE_END_DEFINE_CLASS()


NFE_DEFINE_CLASS(NFE::RT::ColorMaterialParameter)
{
    NFE_CLASS_MEMBER(mBaseValue).NonNull();
    NFE_CLASS_MEMBER(mTexture);
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

using namespace Math;

ColorMaterialParameter::ColorMaterialParameter()
    : ColorMaterialParameter(HdrColorRGB())
{
}

ColorMaterialParameter::ColorMaterialParameter(const HdrColorRGB& baseValue)
{
    NFE_ASSERT(baseValue.IsValid(), "");
    mBaseValue = Common::MakeSharedPtr<ColorRGB>(baseValue);
}

ColorMaterialParameter::~ColorMaterialParameter() = default;

bool ColorMaterialParameter::IsValid() const
{
    return mBaseValue && mBaseValue->IsValid();
}

void ColorMaterialParameter::SetBaseValue(const ColorPtr& baseValueColor)
{
    NFE_ASSERT(baseValueColor && baseValueColor->IsValid(), "");
    mBaseValue = baseValueColor;
}

void ColorMaterialParameter::SetTexture(const TexturePtr& texture)
{
    mTexture = texture;
}

const RayColor ColorMaterialParameter::Evaluate(const Vec4f& uv, const Wavelength& wavelength) const
{
    RayColor color = mBaseValue->Resolve(wavelength);

    if (mTexture)
    {
        const Vec4f textureColor = Vec4f::Max(Vec4f::Zero(), mTexture->Evaluate(uv));
        color *= RayColor::ResolveRGB(wavelength, textureColor);
    }

    return color;
}

} // namespace RT
} // namespace NFE
