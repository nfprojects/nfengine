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
 * Class representing master mixing channel (output to the hardware)
 */
class NFE_AUDIO_WINDOWS_API MasterChannel
    : public IChannel
{
public:

private:
    IXAudio2MasteringVoice* mMasteringVoice;
};


} // namespace Audio
} // namespace NFE
