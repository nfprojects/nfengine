#pragma once

#include "../Raytracer.h"
#include "../../nfCommon/Math/Random.hpp"
#include "../../nfCommon/Containers/DynArray.hpp"

namespace NFE {
namespace RT {

// multidimensional Halton sequence generator
// taken from: https://github.com/llxu2017/halton
class HaltonSequence
{
public:
    static constexpr uint32 MaxDimensions = 4096;
    static constexpr uint32 Width = 64;

    NFE_RAYTRACER_API HaltonSequence();
    NFE_RAYTRACER_API ~HaltonSequence();
    NFE_RAYTRACER_API void Initialize(uint32 mDimensions);

    NFE_FORCE_INLINE uint32 GetNumDimensions() const { return mDimensions; }

    NFE_RAYTRACER_API void NextSample();

    NFE_FORCE_INLINE double GetDouble(uint32 dimension) { return rnd[dimension][0]; }
    NFE_FORCE_INLINE uint32 GetInt(uint32 dimension) { return uint32(rnd[dimension][0] * (double)UINT32_MAX); }

private:
    uint64 Permute(uint32 i, uint8 j);

    void ClearPermutation();
    void InitPrimes();
    void InitStart();
    void InitPowerBuffer();
    void InitExpansion();
    void InitPermutation();

    uint32 mDimensions;
    Common::DynArray<uint64> mStarts;
    Common::DynArray<uint32> mBase;
    Common::DynArray<Common::DynArray<double>> rnd;
    Common::DynArray<Common::DynArray<uint64>> digit;
    Common::DynArray<Common::DynArray<uint64>> mPowerBuffer;
    uint64 **ppm;
    Math::Random mRandom;
};

} // namespace RT
} // namespace NFE
