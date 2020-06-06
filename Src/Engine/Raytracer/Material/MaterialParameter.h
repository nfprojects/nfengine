#pragma once

#include "../Textures/Texture.h"
#include "../Color/Color.h"

namespace NFE {
namespace RT {

class IColor;
using ColorPtr = Common::SharedPtr<IColor>;

class MaterialParameter
{
    NFE_DECLARE_CLASS(MaterialParameter)

public:

    float baseValue = 0.0f;
    TexturePtr texture = nullptr;

    MaterialParameter() = default;

    NFE_FORCE_INLINE MaterialParameter(const float baseValue) : baseValue(baseValue) {}

    NFE_FORCE_INLINE float Evaluate(const Math::Vec4f& uv) const
    {
        float value = baseValue;

        if (texture)
        {
            value = static_cast<float>(value * texture->Evaluate(uv));
        }

        return value;
    };
};

class ColorMaterialParameter
{
    NFE_DECLARE_CLASS(ColorMaterialParameter)

public:

    NFE_RAYTRACER_API ColorMaterialParameter();
    NFE_RAYTRACER_API ColorMaterialParameter(const Math::HdrColorRGB& baseValue);
    NFE_RAYTRACER_API ~ColorMaterialParameter();

    bool IsValid() const;

    NFE_RAYTRACER_API void SetBaseValue(const ColorPtr& baseValueColor);
    NFE_RAYTRACER_API void SetTexture(const TexturePtr& texture);

    const RayColor Evaluate(const Math::Vec4f& uv, const Wavelength& wavelength) const;

private:

    ColorPtr mBaseValue;
    TexturePtr mTexture = nullptr;
};

} // namespace RT
} // namespace NFE
