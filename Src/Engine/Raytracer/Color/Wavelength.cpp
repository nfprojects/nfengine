#include "PCH.h"
#include "Wavelength.h"

namespace NFE {
namespace RT {

using namespace Math;

#ifdef NFE_ENABLE_SPECTRAL_RENDERING

void Wavelength::InitRange(uint32 binId, uint32 numBins)
{
    NFE_ASSERT(binId < numBins, "");

    float offset = 1.0f / static_cast<float>(numBins * NumComponents);

    value = Vec8f(static_cast<float>(binId) / static_cast<float>(numBins));
    value += Vec8f(0.0f, 1.0f * offset, 2.0f * offset, 3.0f * offset, 4.0f * offset, 5.0f * offset, 6.0f * offset, 7.0f * offset);

    // make multi wavelength
    isSingle = false;
}

void Wavelength::Randomize(float u)
{
    constexpr float offset = 1.0f / static_cast<float>(NumComponents);

    value = Vec8f(u); // "hero" wavelength
    value += Vec8f(0.0f, 1.0f * offset, 2.0f * offset, 3.0f * offset, 4.0f * offset, 5.0f * offset, 6.0f * offset, 7.0f * offset);
    value = Vec8f::Fmod1(value);
    value *= 0.99999f; // make sure the value does not exceed 1.0f

    // make multi wavelength
    isSingle = false;
}

const Wavelength::ValueType Wavelength::SampleSpectrum(const float* data, const uint32 numValues) const
{
    const Vec8f scaledWavelengths = value * static_cast<float>(numValues - 1);
    const Vec8i indices = Vec8i::Convert(scaledWavelengths);
    const Vec8f weights = scaledWavelengths - indices.ConvertToFloat();

    const Vec8f a = Gather8(data, indices);
    const Vec8f b = Gather8(data + 1, indices);

    ValueType result = Vec8f::Lerp(a, b, weights);

    /*
    for (uint32 i = 0; i < Wavelength::NumComponents; ++i)
    {
        assert(wavelength.value[i] >= 0.0f);
        assert(wavelength.value[i] < 1.0f);

        const float w = wavelength.value[i] * static_cast<float>(numValues - 1);
        const uint32 index = static_cast<uint32>(w);
        assert(index >= 0);
        assert(index + 1 < numValues);

        const float weight = w - static_cast<float>(index);
        value[i] = Lerp(data[index], data[index + 1], weight);
    }
    */

    return result;
}

#else // !NFE_ENABLE_SPECTRAL_RENDERING

void Wavelength::InitRange(uint32, uint32)
{
}

void Wavelength::Randomize(float)
{
}

#endif // NFE_ENABLE_SPECTRAL_RENDERING

} // namespace RT
} // namespace NFE
