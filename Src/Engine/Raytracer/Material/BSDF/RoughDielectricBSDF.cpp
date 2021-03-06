#include "PCH.h"
#include "Microfacet.h"
#include "RoughDielectricBSDF.h"
#include "DielectricBSDF.h"
#include "Material/Material.h"
#include "Sampling/GenericSampler.h"
#include "../Common/Math/Utils.hpp"
#include "../Common/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::RoughDielectricBSDF)
    NFE_CLASS_PARENT(NFE::RT::BSDF);
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

bool RoughDielectricBSDF::Sample(SamplingContext& ctx) const
{
    const float NdotV = ctx.outgoingDir.z;
    if (Abs(NdotV) < CosEpsilon)
    {
        return false;
    }

    float ior = ctx.materialParam.IoR;
    bool fallbackToSingleWavelength = false;

    // handle dispersion
#ifdef NFE_ENABLE_SPECTRAL_RENDERING
    if (ctx.material.dispersion.enable)
    {
        const float lambda = 1.0e+6f * (Wavelength::Lower + ctx.wavelength.GetBase() * (Wavelength::Higher - Wavelength::Lower));
        // Cauchy's equation for light dispersion
        const float lambda2 = lambda * lambda;
        const float lambda4 = lambda2 * lambda2;
        ior += ctx.material.dispersion.C / lambda2;
        ior += ctx.material.dispersion.D / lambda4;
        if (!ctx.wavelength.isSingle)
        {
            fallbackToSingleWavelength = true;
            ctx.wavelength.isSingle = true;
        }
    }
#endif // NFE_ENABLE_SPECTRAL_RENDERING

    const float roughness = ctx.materialParam.roughness;

    // fallback to specular event
    if (roughness < SpecularEventRoughnessTreshold)
    {
        return DielectricBSDF().Sample(ctx);
    }

    const Vec3f u = ctx.sampler.GetVec3f();

    // microfacet normal (aka. half vector)
    const Microfacet microfacet(roughness * roughness);
    const Vec4f m = microfacet.Sample(u);
    const float microfacetPdf = microfacet.Pdf(m);
    const float VdotH = Vec4f::Dot3(m, ctx.outgoingDir);

    // compute Fresnel term
    const float F = FresnelDielectric(VdotH, ior);
    const bool reflection = u.z < F;

    if (reflection)
    {
        ctx.outIncomingDir = -Vec4f::Reflect3(ctx.outgoingDir, m);
        ctx.outEventType = GlossyReflectionEvent;
    }
    else // transmission
    {
        ctx.outIncomingDir = Vec4f::Refract3(-ctx.outgoingDir, m, ior);
        ctx.outEventType = GlossyRefractionEvent;
    }

    const float NdotL = ctx.outIncomingDir.z;
    const float LdotH = Vec4f::Dot3(m, ctx.outIncomingDir);

    if ((NdotV * NdotL > 0.0f) != reflection)
    {
        // discard samples that land on wrong surface side
        return false;
    }

    const float D = microfacet.D(m);
    const float G = microfacet.G(NdotV, NdotL);

    ctx.outColor = RayColor(Abs(VdotH) * G * D / (microfacetPdf * Abs(NdotV)));

    if (reflection)
    {
        ctx.outPdf = F * microfacetPdf / (4.0f * Abs(VdotH));
        // Note: reflection is white for dielectrics
    }
    else
    {
        const float eta = NdotV < 0.0f ? ior : 1.0f / ior;
        const float denom = Sqr(eta * VdotH + LdotH);
        ctx.outPdf = (1.0f - F) * microfacetPdf * Abs(LdotH) / denom;
        ctx.outColor *= ctx.materialParam.baseColor;

        //if (!ctx.lightPath)
        //{
        //    ctx.outColor *= Sqr(eta);
        //}
    }

    if (fallbackToSingleWavelength)
    {
        // in case of wavelength-dependent event, switch to single wavelength per ray
        ctx.outColor *= RayColor::SingleWavelengthFallback();
    }

    return true;
}

const RayColor RoughDielectricBSDF::Evaluate(const EvaluationContext& ctx, float* outDirectPdfW, float* outReversePdfW) const
{
    const float NdotV = ctx.outgoingDir.z; // wi
    const float NdotL = -ctx.incomingDir.z; // wo

    if (Abs(NdotV) < CosEpsilon || Abs(NdotL) < CosEpsilon)
    {
        return RayColor::Zero();
    }

    const float roughness = ctx.materialParam.roughness;
    if (roughness < SpecularEventRoughnessTreshold)
    {
        return RayColor::Zero();
    }

    // TODO handle dispersion
    float ior = ctx.materialParam.IoR;
    const float eta = NdotV < 0.0f ? ior : 1.0f / ior;

    const bool reflection = NdotV * NdotL >= 0.0f;

    // microfacet normal
    Vec4f m;
    if (reflection)
    {
        m = ctx.outgoingDir - ctx.incomingDir;
    }
    else
    {
        m = eta * ctx.outgoingDir - ctx.incomingDir;
    }
    m *= Signum(m.z);
    m.Normalize3();

    if (Abs(m.z) < CosEpsilon)
    {
        return RayColor::Zero();
    }

    const float VdotH = Vec4f::Dot3(m, ctx.outgoingDir);
    const float LdotH = Vec4f::Dot3(m, -ctx.incomingDir);

    RayColor color;
    float pdf;

    const Microfacet microfacet(roughness * roughness);
    const float F = FresnelDielectric(VdotH, ior);
    const float D = microfacet.D(m);
    const float G = microfacet.G(NdotV, NdotL);

    if (reflection)
    {
        pdf = F * microfacet.Pdf(m) / (4.0f * Abs(VdotH));
        color = RayColor(F * G * D / (4.0f * Abs(NdotV)));
    }
    else // transmission
    {
        const float denom = Sqr(eta * VdotH + LdotH);
        pdf = (1.0f - F) * microfacet.Pdf(m) * Abs(LdotH) / denom;
        color = RayColor(Abs(VdotH * LdotH) * (1.0f - F) * G * D / (denom * Abs(NdotV)));
    }

    NFE_ASSERT(pdf >= 0.0f, "");

    if (outDirectPdfW)
    {
        *outDirectPdfW = pdf;
    }

    if (outReversePdfW)
    {
        // TODO is this correct for reverse direction?
        *outReversePdfW = pdf;
    }

    return RayColor(color);
}

float RoughDielectricBSDF::Pdf(const EvaluationContext& ctx, PdfDirection dir) const
{
    // TODO is this correct for reverse direction?
    NFE_UNUSED(dir);

    const float NdotV = ctx.outgoingDir.z; // wi
    const float NdotL = -ctx.incomingDir.z; // wo

    if (Abs(NdotV) < CosEpsilon || Abs(NdotL) < CosEpsilon)
    {
        return 0.0f;
    }

    const float roughness = ctx.materialParam.roughness;
    if (roughness < SpecularEventRoughnessTreshold)
    {
        return 0.0f;
    }

    // TODO handle dispersion
    float ior = ctx.materialParam.IoR;
    const float eta = NdotV < 0.0f ? ior : 1.0f / ior;

    const bool reflection = NdotV * NdotL >= 0.0f;

    // microfacet normal
    Vec4f m;
    if (reflection)
    {
        m = ctx.outgoingDir - ctx.incomingDir;
    }
    else
    {
        m = eta * ctx.outgoingDir - ctx.incomingDir;
    }
    m *= Signum(m.z);
    m.Normalize3();

    if (Abs(m.z) < CosEpsilon)
    {
        return 0.0f;
    }

    const float VdotH = Vec4f::Dot3(m, ctx.outgoingDir);
    const float LdotH = Vec4f::Dot3(m, -ctx.incomingDir);

    const Microfacet microfacet(roughness * roughness);
    const float F = FresnelDielectric(VdotH, ior);

    if (reflection)
    {
        return F * microfacet.Pdf(m) / (4.0f * Abs(VdotH));
    }
    else // transmission
    {
        const float denom = Sqr(eta * VdotH + LdotH);
        return (1.0f - F) * microfacet.Pdf(m) * Abs(LdotH) / denom;
    }
}

} // namespace RT
} // namespace NFE
