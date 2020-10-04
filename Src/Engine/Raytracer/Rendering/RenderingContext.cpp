#include "PCH.h"
#include "RenderingContext.h"

namespace NFE {
namespace RT {

RenderingContext::RenderingContext() = default;

RenderingContext::~RenderingContext() = default;

void SpectrumDebugData::Clear()
{
    for (float& x : samples)
    {
        x = 0.0f;
    }
}

void SpectrumDebugData::Accumulate(const RayColor& rayColor, const Wavelength& wavelength)
{
    for (uint32 i = 0; i < Wavelength::NumComponents; ++i)
    {
        const int32 binId = static_cast<int32>(wavelength.value[i] * samples.Size());
        if (binId >= 0 && (uint32)binId < samples.Size())
        {
            samples[binId] += rayColor[i];
        }
    }
}

} // namespace RT
} // namespace NFE
