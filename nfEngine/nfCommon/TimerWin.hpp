/**
 *  @file   TimerWin.hpp
 *  @brief  High resolution timer class declaration for Windows system
 */

#pragma once

#include "nfCommon.h"

namespace NFE {
namespace Common {

class NFCOMMON_API Timer
{
private:
    LARGE_INTEGER mFreq; // frequency of clock
    LARGE_INTEGER mStart, mStop; // start/stop points

public:
    Timer();

    /**
     * Start time measurement
     */
    void Start();

    /**
     * Stop time measurement
     * @return Seconds elapsed since last Start() call
     */
    double Stop();

    //TODO: Pause / Resume
};

} // namespace Common
} // namespace NFE
