#include "PCH.h"
#include "SpotLight.h"
#include "../../Rendering/RenderingContext.h"
#include "../../Rendering/ShadingData.h"
#include "../../../nfCommon/Math/Geometry.hpp"
#include "../../../nfCommon/Math/Transcendental.hpp"
#include "../../../nfCommon/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::SpotLight)
{
    NFE_CLASS_PARENT(NFE::RT::ILight);
    NFE_CLASS_MEMBER(mAngle).Min(0.0f).Max(NFE_MATH_2PI - 0.001f); // TODO degrees
}
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

SpotLight::SpotLight(const Math::HdrColorRGB& color, const float angle)
    : ILight(color)
    , mAngle(angle)
{
    NFE_ASSERT(angle >= 0.0f && angle < NFE_MATH_2PI);

    mCosAngle = cosf(angle);
    mIsDelta = mCosAngle > CosEpsilon;
}

const Box SpotLight::GetBoundingBox() const
{
    return Box(Vector4::Zero());
}

const RayColor SpotLight::Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const
{
    outResult.directionToLight = param.lightToWorld.GetTranslation() - param.intersection.frame.GetTranslation();
    const float sqrDistance = outResult.directionToLight.SqrLength3();

    outResult.directPdfW = sqrDistance;
    outResult.distance = std::sqrt(sqrDistance);
    outResult.directionToLight /= outResult.distance;
    outResult.cosAtLight = 1.0f;
    outResult.emissionPdfW = mIsDelta ? 1.0f : SphereCapPdf(mCosAngle);

    const float angle = Vector4::Dot3(outResult.directionToLight, -VECTOR_Z);
    
    if (angle < mCosAngle)
    {
        return RayColor::Zero();
    }

    // TODO IES profile
    return GetColor()->Resolve(param.wavelength);
}

const RayColor SpotLight::Emit(const EmitParam& param, EmitResult& outResult) const
{
    if (mIsDelta)
    {
        outResult.emissionPdfW = 1.0f;
        outResult.direction = VECTOR_Z;
    }
    else
    {
        const float phi = NFE_MATH_2PI * param.directionSample.y;
        const Vector4 sinCosPhi = SinCos(phi);

        float cosTheta = Lerp(mCosAngle, 1.0f, param.directionSample.x);
        float sinThetaSqr = 1.0f - Sqr(cosTheta);
        float sinTheta = sqrtf(sinThetaSqr);

        // generate ray direction in the cone uniformly
        outResult.direction.x = sinTheta * sinCosPhi.x;
        outResult.direction.y = sinTheta * sinCosPhi.y;
        outResult.direction.z = cosTheta;
        outResult.direction.Normalize3();
        outResult.emissionPdfW = SphereCapPdf(mCosAngle);
    }

    outResult.position = param.lightToWorld.GetTranslation();
    outResult.directPdfA = 1.0f;
    outResult.cosAtLight = 1.0f;

    // TODO IES profile
    return GetColor()->Resolve(param.wavelength);
}

ILight::Flags SpotLight::GetFlags() const
{
    return mIsDelta ? Flags(Flag_IsFinite | Flag_IsDelta) : Flag_IsFinite;
}

} // namespace RT
} // namespace NFE
