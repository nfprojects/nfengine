#include "PCH.h"
#include "PlasticBSDF.h"
#include "Sampling/GenericSampler.h"
#include "../Common/Math/SamplingHelpers.hpp"
#include "../Common/Math/Utils.hpp"
#include "../Common/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::PlasticBSDF)
NFE_CLASS_PARENT(NFE::RT::BSDF);
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

bool PlasticBSDF::Sample(SamplingContext& ctx) const
{
    const float NdotV = ctx.outgoingDir.z;
    if (NdotV < CosEpsilon)
    {
        return false;
    }

    const float ior = ctx.materialParam.IoR;

    const float Fi = FresnelDielectric(NdotV, ior);

    // increase probability of sampling reflection
    // this helps reducing noise in bright reflections
    const float minSpecularWeight = 0.25f;

    const float specularWeight = minSpecularWeight + Fi * (1.0f - minSpecularWeight);
    const float diffuseWeight = (1.0f - Fi) * ctx.materialParam.baseColor.Max();

    // importance sample specular reflectivity
    const float specularProbability = specularWeight / (specularWeight + diffuseWeight);
    const float diffuseProbability = 1.0f - specularProbability;

    const Vec3f u = ctx.sampler.GetVec3f();

    const bool specular = (specularProbability >= 1.0f) || (u.z < specularProbability);

    if (specular)
    {
        ctx.outColor = RayColor(Fi / specularProbability);
        ctx.outIncomingDir = -Vec4f::Reflect3Z(ctx.outgoingDir);
        ctx.outPdf = specularProbability;
        ctx.outEventType = SpecularReflectionEvent;
    }
    else // diffuse reflection
    {
        NFE_ASSERT(diffuseProbability > 0.0f, "");

        ctx.outIncomingDir = SamplingHelpers::GetHemishpereCos(u);
        const float NdotL = ctx.outIncomingDir.z;

        ctx.outPdf = ctx.outIncomingDir.z * NFE_MATH_INV_PI * diffuseProbability;

        const float Fo = FresnelDielectric(NdotL, ior);
        ctx.outColor = ctx.materialParam.baseColor * ((1.0f - Fi) * (1.0f - Fo) / diffuseProbability);

        ctx.outEventType = DiffuseReflectionEvent;
    }

    return true;
}

const RayColor PlasticBSDF::Evaluate(const EvaluationContext& ctx, float* outDirectPdfW, float* outReversePdfW) const
{
    const float NdotV = ctx.outgoingDir.z;
    const float NdotL = -ctx.incomingDir.z;

    if (NdotV < CosEpsilon || NdotL < CosEpsilon)
    {
        return RayColor::Zero();
    }

    const float ior = ctx.materialParam.IoR;

    const float Fi = FresnelDielectric(NdotV, ior);
    const float Fo = FresnelDielectric(NdotL, ior);

    const float specularWeight = Fi;
    const float diffuseWeight = (1.0f - Fi) * ctx.materialParam.baseColor.Max();

    const float specularProbability = specularWeight / (specularWeight + diffuseWeight);
    const float diffuseProbability = 1.0f - specularProbability;

    if (outDirectPdfW)
    {
        // cos-weighted hemisphere distribution
        *outDirectPdfW = NdotL * NFE_MATH_INV_PI * diffuseProbability;
    }

    if (outReversePdfW)
    {
        // cos-weighted hemisphere distribution
        *outReversePdfW = NdotV * NFE_MATH_INV_PI * diffuseProbability;
    }

    return ctx.materialParam.baseColor * (NdotL * NFE_MATH_INV_PI * (1.0f - Fi) * (1.0f - Fo));
}

float PlasticBSDF::Pdf(const EvaluationContext& ctx, PdfDirection dir) const
{
    const float NdotV = ctx.outgoingDir.z;
    const float NdotL = -ctx.incomingDir.z;

    if (NdotV < CosEpsilon || NdotL < CosEpsilon)
    {
        return 0.0f;
    }

    const float Fi = FresnelDielectric(NdotV, ctx.materialParam.IoR);

    const float specularWeight = Fi;
    const float diffuseWeight = (1.0f - Fi) * ctx.materialParam.baseColor.Max();

    const float specularProbability = specularWeight / (specularWeight + diffuseWeight);
    const float diffuseProbability = 1.0f - specularProbability;

    if (dir == ForwardPdf)
    {
        return NdotL * NFE_MATH_INV_PI * diffuseProbability;
    }
    else
    {
        return NdotV * NFE_MATH_INV_PI * diffuseProbability;
    }
}

} // namespace RT
} // namespace NFE
