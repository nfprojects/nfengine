#include "PCH.h"
#include "DielectricBSDF.h"
#include "Material/Material.h"
#include "Sampling/GenericSampler.h"
#include "../Common/Math/Utils.hpp"
#include "../Common/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::DielectricBSDF)
NFE_CLASS_PARENT(NFE::RT::BSDF);
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

bool DielectricBSDF::Sample(SamplingContext& ctx) const
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

    // compute Fresnel term
    const float F = FresnelDielectric(NdotV, ior);

    // increase probability of sampling refraction
    // this helps reducing noise in reflections in dielectrics when refracted ray returns dark color
    const float minReflectionProbability = 0.25f;
    const float reflectionProbability = minReflectionProbability + (1.0f - minReflectionProbability) * F;
    const float refractionProbability = 1.0f - reflectionProbability;

    // sample event
    const float u = ctx.sampler.GetFloat();
    const bool reflection = (reflectionProbability >= 1.0f) || u < reflectionProbability;
    if (reflection) 
    {
        ctx.outIncomingDir = -Vec4f::Reflect3Z(ctx.outgoingDir);
        ctx.outEventType = SpecularReflectionEvent;
    }
    else // transmission
    {
        ctx.outIncomingDir = Vec4f::Refract3(-ctx.outgoingDir, VECTOR_Z, ior);
        ctx.outEventType = SpecularRefractionEvent;
    }

    const float NdotL = ctx.outIncomingDir.z;

    if ((NdotV * NdotL > 0.0f) != reflection)
    {
        // discard samples that land on wrong surface side
        return false;
    }

    if (reflection)
    {
        ctx.outPdf = reflectionProbability;
        ctx.outColor = RayColor::One();

        if (minReflectionProbability > 0.0f)
        {
            ctx.outColor *= F / reflectionProbability;
        }
    }
    else
    {
        ctx.outPdf = refractionProbability;
        ctx.outColor = ctx.materialParam.baseColor;

        if (minReflectionProbability > 0.0f)
        {
            ctx.outColor *= (1.0f - F) / refractionProbability;
        }
    }

    if (fallbackToSingleWavelength)
    {
        // in case of wavelength-dependent event, switch to single wavelength per ray
        ctx.outColor *= RayColor::SingleWavelengthFallback();
    }

    return true;
}

const RayColor DielectricBSDF::Evaluate(const EvaluationContext& ctx, float* outDirectPdfW, float* outReversePdfW) const
{
    NFE_UNUSED(ctx);

    // Dirac delta, assume we cannot hit it

    if (outDirectPdfW)
    {
        *outDirectPdfW = 0.0f;
    }

    if (outReversePdfW)
    {
        *outReversePdfW = 0.0f;
    }

    return RayColor::Zero();
}

float DielectricBSDF::Pdf(const EvaluationContext& ctx, PdfDirection dir) const
{
    NFE_UNUSED(ctx);
    NFE_UNUSED(dir);

    // Dirac delta, assume we cannot hit it
    return 0.0f;
}

} // namespace RT
} // namespace NFE
