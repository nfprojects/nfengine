#pragma once

#include "Texture.h"
#include "../Utils/Memory.h"
#include "../../Common/Math/Plane.hpp"
#include "../../Common/Math/HdrColor.hpp"

namespace NFE {
namespace RT {

class NFE_ALIGN(16) GradientTexture
    : public ITexture
{
    NFE_DECLARE_POLYMORPHIC_CLASS(GradientTexture)

public:
    NFE_RAYTRACER_API GradientTexture(const Math::Vec4f& colorA, const Math::Vec4f& colorB, const Math::Plane& plane, float planeDistance);

    virtual const char* GetName() const override;
    virtual const Math::Vec4f Evaluate(const Math::Vec4f& coords) const override;

private:
    Math::HdrColorRGBA mColorA;
    Math::HdrColorRGBA mColorB;

    Math::Plane mPlane;
    float mDistance;
    float mInvDistance;
};

} // namespace RT
} // namespace NFE
