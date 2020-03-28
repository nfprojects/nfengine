#include "PCH.h"
#include "GradientTexture.h"
#include "../../Common/Math/ColorHelpers.hpp"
#include "../../Common/Math/Math.hpp"

namespace NFE {
namespace RT {

using namespace Math;

GradientTexture::GradientTexture(const Vec4f& colorA, const Vec4f& colorB, const Plane& plane, float planeDistance)
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

const Vec4f GradientTexture::Evaluate(const Vec4f& coords) const
{
    const float t = Saturate(mPlane.PointDistance(coords) * mInvDistance);
    return Vec4f::Lerp(mColorA, mColorB, t);
}

} // namespace RT
} // namespace NFE
