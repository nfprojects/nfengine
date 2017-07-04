/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 */

#pragma once

#include "AudioWindows.hpp"
#include "Channel.hpp"


namespace NFE {
namespace Audio {


/**
 * Class representing audio source.
 */
class NFE_AUDIO_WINDOWS_API SourceChannel : public IChannel
{
public:
    uint32 GetSampleRate();

private:
    IXAudio2SourceVoice* mSourceVoice;
};


} // namespace Audio
} // namespace NFE
