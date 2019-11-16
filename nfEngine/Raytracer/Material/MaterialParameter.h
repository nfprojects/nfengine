#pragma once

#include "../Textures/Texture.h"
#include "../../nfCommon/Math/HdrColor.hpp"

namespace NFE {
namespace RT {

class MaterialParameter
{
    NFE_DECLARE_CLASS(MaterialParameter);

public:

    float baseValue = 0.0f;
    TexturePtr texture = nullptr;

    MaterialParameter() = default;

    NFE_FORCE_INLINE MaterialParameter(const float baseValue) : baseValue(baseValue) {}

    NFE_FORCE_INLINE float Evaluate(const Math::Vector4& uv) const
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
    NFE_DECLARE_CLASS(ColorMaterialParameter);

public:

    Math::HdrColorRGB baseValue;
    TexturePtr texture = nullptr;

    ColorMaterialParameter() = default;

    NFE_FORCE_INLINE ColorMaterialParameter(const Math::HdrColorRGB baseValue) : baseValue(baseValue) {}

    NFE_FORCE_INLINE const Math::Vector4 Evaluate(const Math::Vector4& uv) const
    {
        Math::Vector4 value = baseValue.ToVector4();

        if (texture)
        {
            value *= texture->Evaluate(uv);
        }

        return value;
    };
};

} // namespace RT
} // namespace NFE
