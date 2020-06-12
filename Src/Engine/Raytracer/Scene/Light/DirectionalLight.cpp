#include "PCH.h"
#include "DirectionalLight.h"
#include "../../Rendering/RenderingContext.h"
#include "../../../Common/Math/Geometry.hpp"
#include "../../../Common/Math/SamplingHelpers.hpp"
#include "../../../Common/Math/Transcendental.hpp"
#include "../../../Common/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::DirectionalLight)
{
    NFE_CLASS_PARENT(NFE::RT::ILight);
    // NFE_CLASS_MEMBER(mAngle); // TODO
}
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

static constexpr const float SceneRadius = 30.0f; // TODO

DirectionalLight::DirectionalLight(const Math::HdrColorRGB& color, const float angle)
    : ILight(color)
{
    NFE_ASSERT(angle >= 0.0f && angle < NFE_MATH_2PI, "");
    mCosAngle = cosf(angle);
    mIsDelta = mCosAngle > CosEpsilon;
}

const Box DirectionalLight::GetBoundingBox() const
{
    return Box::Full();
}

const Vec4f DirectionalLight::SampleDirection(const Vec2f sample, float& outPdf) const
{
    Vec4f sampledDirection = Vec4f::Zero();

    if (mIsDelta)
    {
        outPdf = 1.0f;
        sampledDirection = VECTOR_Z; 
    }
    else
    {
        outPdf = SphereCapPdf(mCosAngle);

        const float phi = NFE_MATH_2PI * sample.y;
        const Vec4f sinCosPhi = SinCos(phi);

        float cosTheta = Lerp(mCosAngle, 1.0f, sample.x);
        float sinThetaSqr = 1.0f - Sqr(cosTheta);
        float sinTheta = sqrtf(sinThetaSqr);

        // generate ray direction in the cone uniformly
        sampledDirection.x = sinTheta * sinCosPhi.x;
        sampledDirection.y = sinTheta * sinCosPhi.y;
        sampledDirection.z = cosTheta;
        sampledDirection.Normalize3();

        NFE_ASSERT(sampledDirection.IsValid(), "");
    }

    return sampledDirection;
}

const RayColor DirectionalLight::Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const
{
    const Vec4f sampledDirectionLocalSpace = SampleDirection(param.sample, outResult.directPdfW);

    outResult.directionToLight = param.lightToWorld.TransformVectorNeg(sampledDirectionLocalSpace);
    outResult.emissionPdfW = outResult.directPdfW * UniformCirclePdf(SceneRadius);
    outResult.cosAtLight = 1.0f;
    outResult.distance = FLT_MAX;

    return GetColor()->Resolve(param.wavelength);
}

const RayColor DirectionalLight::GetRadiance(const RadianceParam& param, float* outDirectPdfA, float* outEmissionPdfW) const
{
    if (mIsDelta)
    {
        // can't hit delta light
        return RayColor::Zero();
    }

    if (Vec4f::Dot3(param.ray.dir, VECTOR_Z) > -mCosAngle)
    {
        return RayColor::Zero();
    }

    const float directPdf = SphereCapPdf(mCosAngle);

    if (outDirectPdfA)
    {
        *outDirectPdfA = directPdf;
    }

    if (outEmissionPdfW)
    {
        *outEmissionPdfW = directPdf * UniformCirclePdf(SceneRadius);
    }

    return GetColor()->Resolve(param.context.wavelength);
}

const RayColor DirectionalLight::Emit(const EmitParam& param, EmitResult& outResult) const
{
    const Vec4f sampledDirectionLocalSpace = SampleDirection(param.directionSample, outResult.directPdfA);

    outResult.direction = param.lightToWorld.TransformVector(-sampledDirectionLocalSpace);

    // generate random origin
    const Vec4f uv = SamplingHelpers::GetCircle(param.positionSample);
    outResult.position = Vec4f(uv.x, uv.y, -1.0f) * SceneRadius;

    outResult.cosAtLight = 1.0f;
    outResult.emissionPdfW = outResult.directPdfA * UniformCirclePdf(SceneRadius);

    return GetColor()->Resolve(param.wavelength);
}

ILight::Flags DirectionalLight::GetFlags() const
{
    return mIsDelta ? Flag_IsDelta : Flag_None;
}

} // namespace RT
} // namespace NFE
