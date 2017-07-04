/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration nfAudioWindows project public functions
 */

#pragma once


#include "../../nfCommon/nfCommon.hpp"

// TODO remove
#include <xaudio2.h>

#ifdef NFE_AUDIO_WINDOWS_EXPORTS
#define NFE_AUDIO_WINDOWS_API __declspec(dllexport)
#else
#define NFE_AUDIO_WINDOWS_API __declspec(dllimport)
#endif


namespace NFE {
namespace Audio {


} // namespace Audio
} // namespace NFE
