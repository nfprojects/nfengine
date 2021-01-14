#include "PCH.h"
#include "MetalBSDF.h"
#include "../Material.h"
#include "../Common/Math/Utils.hpp"
#include "../Common/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::MetalBSDF)
NFE_CLASS_PARENT(NFE::RT::BSDF);
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

bool MetalBSDF::Sample(SamplingContext& ctx) const
{
    const float NdotV = ctx.outgoingDir.z;
    if (NdotV < CosEpsilon)
    {
        return false;
    }

    const RayColor F = ctx.material.EvaluateMetalFresnel(NdotV, ctx.wavelength);

    ctx.outColor = ctx.materialParam.baseColor * F;
    ctx.outIncomingDir = -Vec4f::Reflect3Z(ctx.outgoingDir);
    ctx.outPdf = 1.0f;
    ctx.outEventType = SpecularReflectionEvent;

    return true;
}

const RayColor MetalBSDF::Evaluate(const EvaluationContext& ctx, float* outDirectPdfW, float* outReversePdfW) const
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

float MetalBSDF::Pdf(const EvaluationContext& ctx, PdfDirection dir) const
{
    NFE_UNUSED(ctx);
    NFE_UNUSED(dir);

    // Dirac delta, assume we cannot hit it
    return 0.0f;
}

} // namespace RT
} // namespace NFE
