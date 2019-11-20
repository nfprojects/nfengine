#include "PCH.h"
#include "GradientTexture.h"
#include "Color/ColorHelpers.h"
#include "../../nfCommon/Math/Math.hpp"

namespace NFE {
namespace RT {

using namespace Math;

GradientTexture::GradientTexture(const Vector4& colorA, const Vector4& colorB, const Plane& plane, float planeDistance)
    : mColorA(colorA)
    , mColorB(colorB)
    , mPlane(plane)
    , mDistance(planeDistance)
{
    NFE_ASSERT(colorA.IsValid());
    NFE_ASSERT(colorB.IsValid());
    NFE_ASSERT(plane.IsValid());
    NFE_ASSERT(planeDistance > 0.0f);

    mInvDistance = 1.0f / planeDistance;
}

const char* GradientTexture::GetName() const
{
    return "gradient";
}

const Vector4 GradientTexture::Evaluate(const Vector4& coords) const
{
    const float t = Saturate(mPlane.PointDistance(coords) * mInvDistance);
    return Vector4::Lerp(mColorA, mColorB, t);
}

} // namespace RT
} // namespace NFE
