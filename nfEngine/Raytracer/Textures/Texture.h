#pragma once

#include "../Raytracer.h"
#include "../../nfCommon/Math/Vector4.hpp"
#include "../../nfCommon/Containers/SharedPtr.hpp"
#include "../../nfCommon/Memory/Aligned.hpp"

namespace NFE {
namespace RT {

/**
 * Class representing 2D texture.
 */
class ITexture : public Common::Aligned<16>
{
public:
    NFE_RAYTRACER_API virtual ~ITexture();

    // get human-readable description
    virtual const char* GetName() const = 0;

    // evaluate texture color at given coordinates
    virtual const Math::Vector4 Evaluate(const Math::Vector4& coords) const = 0;

    // generate random sample on the texture
    virtual const Math::Vector4 Sample(const Math::Float2 u, Math::Vector4& outCoords, float* outPdf = nullptr) const = 0;

    // must be called before using Sample() method
    virtual bool MakeSamplable();

    // check if the texture is samplable (if it's not, calling Sample is illegal)
    virtual bool IsSamplable() const;
};

using TexturePtr = Common::SharedPtr<ITexture>;

} // namespace RT
} // namespace NFE
