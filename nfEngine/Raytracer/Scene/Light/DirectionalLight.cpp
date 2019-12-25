#include "PCH.h"
#include "DirectionalLight.h"
#include "../Camera.h"
#include "../../Rendering/RenderingContext.h"
#include "../../Rendering/ShadingData.h"
#include "../../../nfCommon/Math/Geometry.hpp"
#include "../../../nfCommon/Math/SamplingHelpers.hpp"
#include "../../../nfCommon/Math/Transcendental.hpp"
#include "../../../nfCommon/Reflection/ReflectionClassDefine.hpp"


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
    NFE_ASSERT(angle >= 0.0f && angle < NFE_MATH_2PI);
    mCosAngle = cosf(angle);
    mIsDelta = mCosAngle > CosEpsilon;
}

const Box DirectionalLight::GetBoundingBox() const
{
    return Box::Full();
}

bool DirectionalLight::TestRayHit(const Math::Ray& ray, float& outDistance) const
{
    if (!mIsDelta)
    {
        if (Vector4::Dot3(ray.dir, VECTOR_Z) < -mCosAngle)
        {
            outDistance = FLT_MAX;
            return true;
        }
    }

    return false;
}

const Vector4 DirectionalLight::SampleDirection(const Float2 sample, float& outPdf) const
{
    Vector4 sampledDirection = Vector4::Zero();

    if (mIsDelta)
    {
        outPdf = 1.0f;
        sampledDirection = VECTOR_Z; 
    }
    else
    {
        outPdf = SphereCapPdf(mCosAngle);

        const float phi = NFE_MATH_2PI * sample.y;
        const Vector4 sinCosPhi = SinCos(phi);

        float cosTheta = Lerp(mCosAngle, 1.0f, sample.x);
        float sinThetaSqr = 1.0f - Sqr(cosTheta);
        float sinTheta = sqrtf(sinThetaSqr);

        // generate ray direction in the cone uniformly
        sampledDirection.x = sinTheta * sinCosPhi.x;
        sampledDirection.y = sinTheta * sinCosPhi.y;
        sampledDirection.z = cosTheta;
        sampledDirection.Normalize3();

        NFE_ASSERT(sampledDirection.IsValid());
    }

    return sampledDirection;
}

const RayColor DirectionalLight::Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const
{
    const Vector4 sampledDirectionLocalSpace = SampleDirection(param.sample, outResult.directPdfW);

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

    if (Vector4::Dot3(param.ray.dir, VECTOR_Z) > -mCosAngle)
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
    const Vector4 sampledDirectionLocalSpace = SampleDirection(param.directionSample, outResult.directPdfA);

    outResult.direction = param.lightToWorld.TransformVector(-sampledDirectionLocalSpace);

    // generate random origin
    const Vector4 uv = SamplingHelpers::GetCircle(param.positionSample);
    outResult.position = Vector4(uv.x, uv.y, -1.0f) * SceneRadius;

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
