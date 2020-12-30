#include "PCH.h"
#include "GradientTexture.h"
#include "../Common/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::GradientTexture)
{
    NFE_CLASS_PARENT(NFE::RT::ITexture);
    NFE_CLASS_MEMBER(mColorA);
    NFE_CLASS_MEMBER(mColorB);
    //NFE_CLASS_MEMBER(mPlane);
    //NFE_CLASS_MEMBER(mDistance);
}
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

GradientTexture::GradientTexture(const Vec4f& colorA, const Vec4f& colorB, const Plane& plane, float planeDistance)
    : mColorA(colorA)
    , mColorB(colorB)
    , mPlane(plane)
    , mDistance(planeDistance)
{
    NFE_ASSERT(colorA.IsValid(), "");
    NFE_ASSERT(colorB.IsValid(), "");
    NFE_ASSERT(plane.IsValid(), "");
    NFE_ASSERT(planeDistance > 0.0f, "");

    mInvDistance = 1.0f / planeDistance;
}

const char* GradientTexture::GetName() const
{
    return "gradient";
}

const Vec4f GradientTexture::Evaluate(const Vec4f& coords) const
{
    const float t = Saturate(mPlane.PointDistance(coords) * mInvDistance);
    return Vec4f::Lerp(mColorA.ToVec4f(), mColorB.ToVec4f(), t);
}

} // namespace RT
} // namespace NFE
