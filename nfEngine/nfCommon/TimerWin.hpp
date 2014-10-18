/**
 *  @file   TimerWin.hpp
 *  @brief  High resolution timer class declaration for Windows system
 */

#pragma once

#include "nfCommon.hpp"


namespace NFE {
namespace Common {

class NFCOMMON_API Timer
{
private:
    LARGE_INTEGER mStart; // start point

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
