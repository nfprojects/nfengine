#include "PCH.h"
#include "Microfacet.h"
#include "RoughMetalBSDF.h"
#include "MetalBSDF.h"
#include "../Material.h"
#include "Sampling/GenericSampler.h"
#include "../Common/Math/Utils.hpp"
#include "../Common/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::RoughMetalBSDF)
    NFE_CLASS_PARENT(NFE::RT::BSDF);
    NFE_CLASS_MEMBER(mUseMultiscatter);
    NFE_CLASS_MEMBER(mMaxScatteringOrder).Min(1).Max(32);
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

bool RoughMetalBSDF::Sample(SamplingContext& ctx) const
{
    const float roughness = ctx.materialParam.roughness;

    // fallback to specular event
    if (roughness < SpecularEventRoughnessTreshold)
    {
        MetalBSDF smoothBsdf;
        return smoothBsdf.Sample(ctx);
    }

    const float NdotV = ctx.outgoingDir.z;
    if (NdotV < CosEpsilon)
    {
        return false;
    }

    // microfacet normal (aka. half vector)
    const Microfacet microfacet(roughness * roughness, ctx.materialParam.roughnessAnisotropy);

    if (mUseMultiscatter)
    {
        // random walk
        uint32 scatteringOrder = 0;
        RayColor throughput = RayColor::One();
        Vec4f wr = -ctx.outgoingDir;
        float hr = 1.0f + microfacet.HeightDistribution_C1(0.999f);

        while (scatteringOrder <= mMaxScatteringOrder)
        {
            // next height
            const float prevHr = hr;
            const float u = Clamp(ctx.sampler.GetFloat(), 0.00001f, 0.99999f);
            hr = microfacet.SampleHeight(wr, hr, u);
            NFE_ASSERT(!IsNaN(hr), "");

            // leave the microsurface?
            if (hr == FLT_MAX)
            {
                break;
            }

            // sample microfacet normal
            const float u1 = ctx.sampler.GetFloat();
            const float u2 = ctx.sampler.GetFloat();
            const Vec4f wm = microfacet.SampleD(-wr, u1, u2);
            const float VdotH = Vec4f::Dot3(wm, -wr);

            // next direction
            wr = Vec4f::Reflect3(wr, wm);

            throughput *= ctx.materialParam.baseColor;
            throughput *= ctx.material.EvaluateMetalFresnel(VdotH, ctx.wavelength);

            scatteringOrder++;
        }

        // not enough scatter events
        if (scatteringOrder > mMaxScatteringOrder)
        {
            return false;
        }

        NFE_ASSERT(wr.z >= 0.0f, "");

        ctx.outIncomingDir = wr;
        ctx.outColor = throughput;
        ctx.outEventType = GlossyReflectionEvent;

        // use single-scatter PDF, should be good enough
        const Vec4f m = (ctx.outgoingDir + wr).Normalized3();
        const float VdotH = Vec4f::Dot3(m, ctx.outgoingDir);
        ctx.outPdf = microfacet.Pdf(m) / (4.0f * VdotH);

        NFE_ASSERT(IsValid(ctx.outPdf), "");
    }
    else
    {
        const Vec4f m = microfacet.Sample(ctx.sampler.GetVec2f());

        // compute reflected direction
        ctx.outIncomingDir = -Vec4f::Reflect3(ctx.outgoingDir, m);
        if (ctx.outIncomingDir.z < CosEpsilon)
        {
            return false;
        }

        const float NdotL = ctx.outIncomingDir.z;
        const float VdotH = Vec4f::Dot3(m, ctx.outgoingDir);

        const float pdf = microfacet.Pdf(m);
        const float D = microfacet.D(m);
        const float G = microfacet.G(NdotV, NdotL);

        const RayColor F = ctx.material.EvaluateMetalFresnel(VdotH, ctx.wavelength);

        ctx.outPdf = pdf / (4.0f * VdotH);
        ctx.outColor = ctx.materialParam.baseColor * F * RayColor(VdotH * G * D / (pdf * NdotV));
        ctx.outEventType = GlossyReflectionEvent;
    }

    return true;
}

const RayColor RoughMetalBSDF::Evaluate(const EvaluationContext& ctx, float* outDirectPdfW, float* outReversePdfW) const
{
    const float roughness = ctx.materialParam.roughness;

    // fallback to specular event
    if (roughness < SpecularEventRoughnessTreshold)
    {
        MetalBSDF smoothBsdf;
        return smoothBsdf.Evaluate(ctx, outDirectPdfW);
    }

    // microfacet normal
    const Vec4f m = (ctx.outgoingDir - ctx.incomingDir).Normalized3();

    const float NdotV = ctx.outgoingDir.z;
    const float NdotL = -ctx.incomingDir.z;
    const float VdotH = Vec4f::Dot3(m, ctx.outgoingDir);

    // clip the function
    if (NdotV < CosEpsilon || NdotL < CosEpsilon || VdotH < CosEpsilon)
    {
        return RayColor::Zero();
    }

    const Microfacet microfacet(roughness * roughness, ctx.materialParam.roughnessAnisotropy);

    if (outDirectPdfW || outReversePdfW)
    {
        const float pdf = microfacet.Pdf(m) / (4.0f * VdotH);

        if (outDirectPdfW)
        {
            *outDirectPdfW = pdf;
        }

        if (outReversePdfW)
        {
            *outReversePdfW = pdf;
        }
    }

    if (mUseMultiscatter)
    {
        // init
        const Vec4f wi = -ctx.incomingDir;
        Vec4f wr = -ctx.outgoingDir;
        float hr = 1.0f + microfacet.HeightDistribution_InvC1(0.999f);

        RayColor throughput = RayColor::One();
        RayColor sum = RayColor::Zero();

        // random walk
        for (uint32 i = 0; i <= mMaxScatteringOrder; ++i)
        {
            // next height
            const float u = Clamp(ctx.sampler.GetFloat(), 0.00001f, 0.99999f);
            hr = microfacet.SampleHeight(wr, hr, u);

            // leave the microsurface?
            if (hr == FLT_MAX)
                break;

            // next event estimation
            const float phasefunction = microfacet.EvalPhaseFunction(-wr, wi);
            const float shadowing = microfacet.G1(wi, hr);
            const float I = phasefunction * shadowing;

            // sample microfacet normal
            const float u1 = ctx.sampler.GetFloat();
            const float u2 = ctx.sampler.GetFloat();
            const Vec4f wm = microfacet.SampleD(-wr, u1, u2);
            const float VdotM = Vec4f::Dot3(wm, -wr);

            // next direction
            wr = Vec4f::Reflect3(wr, wm);

            throughput *= ctx.materialParam.baseColor;
            throughput *= ctx.material.EvaluateMetalFresnel(VdotM, ctx.wavelength);

            if (!IsInfinity(I))
            {
                sum += throughput * I;
            }

            NFE_ASSERT(!IsNaN(hr));
        }

        return sum;
    }
    else
    {
        const float D = microfacet.D(m);
        const float G = microfacet.G(NdotV, NdotL);
        const RayColor F = ctx.material.EvaluateMetalFresnel(VdotH, ctx.wavelength);

        return ctx.materialParam.baseColor * F * RayColor(G * D / (4.0f * NdotV));
    }
}

float RoughMetalBSDF::Pdf(const EvaluationContext& ctx, PdfDirection dir) const
{
    NFE_UNUSED(dir);

    const float roughness = ctx.materialParam.roughness;

    // fallback to specular event
    if (roughness < SpecularEventRoughnessTreshold)
    {
        return 0.0f;
    }

    // microfacet normal
    const Vec4f m = (ctx.outgoingDir - ctx.incomingDir).Normalized3();

    const float NdotV = ctx.outgoingDir.z;
    const float NdotL = -ctx.incomingDir.z;
    const float VdotH = Vec4f::Dot3(m, ctx.outgoingDir);

    // clip the function
    if (NdotV < CosEpsilon || NdotL < CosEpsilon || VdotH < CosEpsilon)
    {
        return 0.0f;
    }

    const Microfacet microfacet(roughness * roughness, ctx.materialParam.roughnessAnisotropy);

    return microfacet.Pdf(m) / (4.0f * VdotH);
}

} // namespace RT
} // namespace NFE
