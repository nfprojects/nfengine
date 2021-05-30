#include "PCH.h"
#include "Microfacet.h"
#include "RoughPlasticBSDF.h"
#include "PlasticBSDF.h"
#include "Sampling/GenericSampler.h"
#include "../Material.h"
#include "../Common/Math/Utils.hpp"
#include "../Common/Math/SamplingHelpers.hpp"
#include "../Common/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::RoughPlasticBSDF)
NFE_CLASS_PARENT(NFE::RT::BSDF);
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

bool RoughPlasticBSDF::Sample(SamplingContext& ctx) const
{
    const float NdotV = ctx.outgoingDir.z;
    if (NdotV < CosEpsilon)
    {
        return false;
    }

    const float roughness = ctx.materialParam.roughness;

    // fallback to specular event
    if (roughness < SpecularEventRoughnessTreshold)
    {
        return PlasticBSDF().Sample(ctx);
    }

    const Vec3f u = ctx.sampler.GetVec3f();

    const float ior = ctx.materialParam.IoR;

    const float Fi = FresnelDielectric(NdotV, ior);

    const float specularWeight = Fi;
    const float diffuseWeight = (1.0f - Fi) * ctx.materialParam.baseColor.Max();
    NFE_ASSERT(diffuseWeight >= 0.0f && diffuseWeight <= 1.0f, "");

    // importance sample specular reflectivity
    const float specularProbability = specularWeight / (specularWeight + diffuseWeight);
    const float diffuseProbability = 1.0f - specularProbability;
    const bool specular = u.z < specularProbability;

    if (specular)
    {
        // microfacet normal (aka. half vector)
        const Microfacet microfacet(roughness * roughness);
        const Vec4f m = microfacet.Sample(u);

        // compute reflected direction
        ctx.outIncomingDir = -Vec4f::Reflect3(ctx.outgoingDir, m);

        const float NdotL = ctx.outIncomingDir.z;
        const float VdotH = Vec4f::Dot3(m, ctx.outgoingDir);

        if (NdotL < CosEpsilon || VdotH < CosEpsilon)
        {
            return false;
        }

        const float pdf = microfacet.Pdf(m);
        const float D = microfacet.D(m);
        const float G = microfacet.G(NdotV, NdotL);
        const float F = FresnelDielectric(VdotH, ctx.material.IoR);

        ctx.outPdf = pdf / (4.0f * VdotH) * specularProbability;
        ctx.outColor = RayColor(VdotH * F * G * D / (pdf * NdotV * specularProbability));
        ctx.outEventType = GlossyReflectionEvent;

        NFE_ASSERT(ctx.outPdf > 0.0f, "");
        NFE_ASSERT(ctx.outColor.IsValid(), "");
    }
    else // diffuse reflection
    {
        ctx.outIncomingDir = SamplingHelpers::GetHemishpereCos(u);
        const float NdotL = ctx.outIncomingDir.z;

        ctx.outPdf = ctx.outIncomingDir.z * NFE_MATH_INV_PI * diffuseProbability;

        const float Fo = FresnelDielectric(NdotL, ior);
        ctx.outColor = ctx.materialParam.baseColor * ((1.0f - Fi) * (1.0f - Fo) / diffuseProbability);

        ctx.outEventType = DiffuseReflectionEvent;
    }

    return true;
}

const RayColor RoughPlasticBSDF::Evaluate(const EvaluationContext& ctx, float* outDirectPdfW, float* outReversePdfW) const
{
    const float roughness = ctx.materialParam.roughness;

    // fallback to specular event
    if (roughness < SpecularEventRoughnessTreshold)
    {
        return PlasticBSDF().Evaluate(ctx, outDirectPdfW);
    }

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
    NFE_ASSERT(diffuseWeight >= 0.0f && diffuseWeight <= 1.0f, "");

    const float specularProbability = specularWeight / (specularWeight + diffuseWeight);
    const float diffuseProbability = 1.0f - specularProbability;

    float diffusePdf = NdotL * NFE_MATH_INV_PI; // cos-weighted hemisphere distribution
    float diffuseReversePdf = NdotV * NFE_MATH_INV_PI;
    float specularPdf = 0.0f;

    RayColor diffuseTerm = ctx.materialParam.baseColor * (NdotL * NFE_MATH_INV_PI * (1.0f - Fi) * (1.0f - Fo));
    RayColor specularTerm = RayColor::Zero();

    {
        // microfacet normal
        const Vec4f m = (ctx.outgoingDir - ctx.incomingDir).Normalized3();
        const float VdotH = Vec4f::Dot3(m, ctx.outgoingDir);

        // clip the function
        if (VdotH >= CosEpsilon)
        {
            const Microfacet microfacet(roughness * roughness);
            const float D = microfacet.D(m);
            const float G = microfacet.G(NdotV, NdotL);
            const float F = FresnelDielectric(VdotH, ctx.material.IoR);

            specularPdf = microfacet.Pdf(m) / (4.0f * VdotH);
            specularTerm = RayColor(F * G * D / (4.0f * NdotV));
        }
    }

    if (outDirectPdfW)
    {
        *outDirectPdfW = diffusePdf * diffuseProbability + specularPdf * specularProbability;
    }

    if (outReversePdfW)
    {
        *outReversePdfW = diffuseReversePdf * diffuseProbability + specularPdf * specularProbability;
    }

    return diffuseTerm + specularTerm;
}

float RoughPlasticBSDF::Pdf(const EvaluationContext& ctx, PdfDirection dir) const
{
    const float roughness = ctx.materialParam.roughness;

    // fallback to specular event
    if (roughness < SpecularEventRoughnessTreshold)
    {
        return PlasticBSDF().Pdf(ctx, dir);
    }

    const float NdotV = ctx.outgoingDir.z;
    const float NdotL = -ctx.incomingDir.z;

    if (NdotV < CosEpsilon || NdotL < CosEpsilon)
    {
        return 0.0f;
    }

    const float Fi = FresnelDielectric(NdotV, ctx.materialParam.IoR);

    const float specularWeight = Fi;
    const float diffuseWeight = (1.0f - Fi) * ctx.materialParam.baseColor.Max();
    NFE_ASSERT(diffuseWeight >= 0.0f && diffuseWeight <= 1.0f, "");

    const float specularProbability = specularWeight / (specularWeight + diffuseWeight);
    const float diffuseProbability = 1.0f - specularProbability;

    // cos-weighted hemisphere distribution
    float diffusePdf;
    if (dir == ForwardPdf)
    {
        diffusePdf = NdotL * NFE_MATH_INV_PI;
    }
    else
    {
        diffusePdf = NdotV * NFE_MATH_INV_PI;
    }

    float specularPdf = 0.0f;

    // microfacet normal
    const Vec4f m = (ctx.outgoingDir - ctx.incomingDir).Normalized3();
    const float VdotH = Vec4f::Dot3(m, ctx.outgoingDir);

    // clip the function
    if (VdotH >= CosEpsilon)
    {
        const Microfacet microfacet(roughness * roughness);
        specularPdf = microfacet.Pdf(m) / (4.0f * VdotH);
    }

    return diffusePdf * diffuseProbability + specularPdf * specularProbability;
}

} // namespace RT
} // namespace NFE
