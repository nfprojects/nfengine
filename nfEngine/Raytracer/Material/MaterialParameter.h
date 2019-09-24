#pragma once

#include "../Textures/Texture.h"

namespace NFE {
namespace RT {

template<typename T>
struct MaterialParameter
{
    T baseValue = T(1.0f);
    TexturePtr texture = nullptr;

    MaterialParameter() = default;

    NFE_FORCE_INLINE MaterialParameter(const T baseValue)
        : baseValue(baseValue)
    {}

    NFE_FORCE_INLINE const T Evaluate(const Math::Vector4& uv) const
    {
        T value = baseValue;

        if (texture)
        {
            value = static_cast<T>(value * texture->Evaluate(uv));
        }

        return value;
    };
};


} // namespace RT
} // namespace NFE
