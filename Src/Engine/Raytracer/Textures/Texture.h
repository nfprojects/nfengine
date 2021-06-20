#pragma once

#include "../Raytracer.h"
#include "../../Common/Math/Vec4f.hpp"
#include "../../Common/Containers/SharedPtr.hpp"
#include "../../Common/Memory/Aligned.hpp"
#include "../../Common/Reflection/ReflectionClassDeclare.hpp"

namespace NFE {
namespace RT {

enum class SampleDistortion
{
    Uniform,
    Spherical,
};

/**
 * Class representing 2D texture.
 */
class ITexture : public IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ITexture)

public:
    NFE_ALIGNED_CLASS(16)

    NFE_RAYTRACER_API virtual ~ITexture();

    // get human-readable description
    virtual const char* GetName() const = 0;

    // evaluate texture color at given coordinates
    virtual const Math::Vec4f Evaluate(const Math::Vec4f& coords) const = 0;

    // get pdf of sampling given coordinates
    virtual float Pdf(SampleDistortion distortion, const Math::Vec4f& coords) const;

    // generate random sample on the texture
    virtual const Math::Vec4f Sample(
        const Math::Vec3f u,
        Math::Vec4f& outCoords,
        SampleDistortion distortion = SampleDistortion::Uniform,
        float* outPdf = nullptr) const;

    // must be called before using Sample() method
    virtual bool MakeSamplable(SampleDistortion distortion);

    // check if the texture is samplable (if it's not, calling Sample is illegal)
    virtual bool IsSamplable(SampleDistortion distortion) const;
};

using TexturePtr = Common::SharedPtr<ITexture>;

} // namespace RT
} // namespace NFE
