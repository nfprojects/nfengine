/**
    NFEngine project

    \file   Timer.h
    \brief  High resolution timer class declaration.
*/

#pragma once
#include "nfCommon.h"

#include <chrono>

namespace NFE {
namespace Common {

/**
 * High resolution timer
 */
class NFCOMMON_API Timer
{
private:
    typedef std::chrono::high_resolution_clock Clock;

    Clock::time_point mStart;
    static double GetFreqInv();

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
