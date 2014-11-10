/**
 * @file   SoundSample.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Sound sample resource declarations.
 */

#pragma once
#include "Core.hpp"
#include "Resource.hpp"

namespace NFE {
namespace Resource {

/**
 * Sound sample information.
 */
struct SoundInfo
{
    uint32 numChannels;

    // 1 - 8bit signed int, 2 - 16bit signed int, 4 - 32bit float
    uint32 bytesPerSample;

    // samples per second
    uint32 sampleRate;

    uint64 numSamples;
};

/**
 * This class describes sound sample resource.
 */
class CORE_API SoundSample : public ResourceBase
{
    SoundInfo mSoundInfo;

    // if sound type = sample, data is stored here
    void* mSampleData;

    bool OnLoad();
    void OnUnload();

public:
    SoundSample();
    ~SoundSample();

    void Release();
};

} // namespace Resource
} // namespace NFE
