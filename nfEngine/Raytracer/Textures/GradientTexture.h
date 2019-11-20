#pragma once

#include "Texture.h"
#include "../Utils/Memory.h"
#include "../../nfCommon/Math/Plane.hpp"

namespace NFE {
namespace RT {

class NFE_ALIGN(16) GradientTexture
    : public ITexture
{
public:
    NFE_RAYTRACER_API GradientTexture(const Math::Vector4& colorA, const Math::Vector4& colorB, const Math::Plane& plane, float planeDistance);

    virtual const char* GetName() const override;
    virtual const Math::Vector4 Evaluate(const Math::Vector4& coords) const override;

private:
    Math::Vector4 mColorA;
    Math::Vector4 mColorB;

    Math::Plane mPlane;
    float mDistance;
    float mInvDistance;
};

} // namespace RT
} // namespace NFE
