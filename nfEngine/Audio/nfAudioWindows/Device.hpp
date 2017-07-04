/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of audio interface (Windows).
 */

#pragma once

#include "AudioWindows.hpp"


namespace NFE {
namespace Audio {


class ISource;
class SampleSource;
class Listener;
class Master;


class NFE_AUDIO_WINDOWS_API Device
{
public:
    SampleSource* CreateSampleSource();

    /**
     * Create audio master (listener that sends audio to the hardware).
     */
    Master* CreateMaster();


};

} // namespace Audio
} // namespace NFE
