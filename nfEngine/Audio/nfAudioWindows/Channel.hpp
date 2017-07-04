/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 */

#pragma once

#include "AudioWindows.hpp"


namespace NFE {
namespace Audio {


enum class ChannelType
{
    Source,     // External audio source, for example (streamed) sound sample
    Internal,   // Internal mixing (submix) channel
    Master,     // Master channel (output to the hardware)
};

/**
 * Class representing an audio mixer channel.
 * Contains sound source, list of effects and list of sends.
 */
class NFE_AUDIO_WINDOWS_API IChannel
{
public:
    virtual ~IChannel() { }

    /**
     * Begin or resume audio playback.
     */
    virtual bool Play() = 0;

    /**
     * Suspend audio playback.
     */
    virtual bool Pause() = 0;

    /**
     * Add a mixing channel to this source send list.
     * @remarks This can be called only for Source and Internal channels.
     */
    bool AddSend(IChannel* sendChannel);

    /**
     * Remove a mixing channel from this source send list.
     * @remarks This can be called only for Source and Internal channels.
     */
    bool RemoveSend(IChannel* sendChannel);

    /**
     * Set channel volume.
     * All input sends are multiplied by this value.
     */
    void SetVolume(float volume);

    /**
     * Get channel volume.
     */
    float GetVolume();


    // TODO sound effects:
    // delay, reverb, EQ, filter, etc.

    /**
     * Get internal XAudio2 voice.
     */
    virtual IXAudio2Voice* GetInternalVoice() = 0;
};


} // namespace Audio
} // namespace NFE
