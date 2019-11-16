#include "PCH.h"
#include "DiffuseBSDF.h"
#include "../../../nfCommon/Math/SamplingHelpers.hpp"

NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::RT::DiffuseBSDF)
NFE_CLASS_PARENT(NFE::RT::BSDF);
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

bool DiffuseBSDF::Sample(SamplingContext& ctx) const
{
    const float NdotV = ctx.outgoingDir.z;

    if (NdotV < CosEpsilon)
    {
        return false;
    }

    ctx.outIncomingDir = SamplingHelpers::GetHemishpereCos(ctx.sample);
    ctx.outPdf = ctx.outIncomingDir.z * NFE_MATH_INV_PI;
    ctx.outColor = ctx.materialParam.baseColor;
    ctx.outEventType = DiffuseReflectionEvent;

    return true;
}

const RayColor DiffuseBSDF::Evaluate(const EvaluationContext& ctx, float* outDirectPdfW, float* outReversePdfW) const
{
    const float NdotV = ctx.outgoingDir.z;
    const float NdotL = -ctx.incomingDir.z;

    if (NdotV > CosEpsilon && NdotL > CosEpsilon)
    {
        if (outDirectPdfW)
        {
            // cos-weighted hemisphere distribution
            *outDirectPdfW = NdotL * NFE_MATH_INV_PI;
        }

        if (outReversePdfW)
        {
            // cos-weighted hemisphere distribution
            *outReversePdfW = NdotV * NFE_MATH_INV_PI;
        }

        return ctx.materialParam.baseColor * RayColor(NdotL * NFE_MATH_INV_PI);
    }

    return RayColor::Zero();
}

float DiffuseBSDF::Pdf(const EvaluationContext& ctx, PdfDirection dir) const
{
    const float NdotV = ctx.outgoingDir.z;
    const float NdotL = -ctx.incomingDir.z;

    if (NdotV > CosEpsilon && NdotL > CosEpsilon)
    {
        if (dir == ForwardPdf)
        {
            return NdotL * NFE_MATH_INV_PI;
        }
        else
        {
            return NdotV * NFE_MATH_INV_PI;
        }
    }

    return 0.0f;
}

} // namespace RT
} // namespace NFE
