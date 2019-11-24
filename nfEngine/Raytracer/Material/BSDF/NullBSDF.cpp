#include "PCH.h"
#include "NullBSDF.h"
#include "../nfCommon/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::NullBSDF)
NFE_CLASS_PARENT(NFE::RT::BSDF);
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

bool NullBSDF::Sample(SamplingContext& ctx) const
{
    NFE_UNUSED(ctx);

    return false;
}

const RayColor NullBSDF::Evaluate(const EvaluationContext& ctx, float* outDirectPdfW, float* outReversePdfW) const
{
    NFE_UNUSED(ctx);
    NFE_UNUSED(outDirectPdfW);
    NFE_UNUSED(outReversePdfW);

    return RayColor::Zero();
}

float NullBSDF::Pdf(const EvaluationContext& ctx, PdfDirection dir) const
{
    NFE_UNUSED(ctx);
    NFE_UNUSED(dir);

    return 0.0f;
}

} // namespace RT
} // namespace NFE
