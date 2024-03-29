/**
 * @file
 * @author Lookey (costyrra.xl@gmail.com)
 * @brief  High resolution timer class declaration
 */

#pragma once

#include "../nfCommon.hpp"

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#elif defined(__LINUX__) | defined(__linux__)
#include <time.h>
#endif

namespace NFE {
namespace Common {

/**
 * Class used to manage high resolution time measurement.
 *
 * API provided by this class is designed to measure time difference between two time points
 * specified by user with Start() and Stop() calls. Time measurement is done with highest precision
 * available on the system.
 */
class NFCOMMON_API Timer
{
private:
#if defined(WIN32)
    LARGE_INTEGER mStart; // start point
#elif defined(__LINUX__) | defined(__linux__)
    struct timespec mStart;
#endif // defined(WIN32)

public:

    static const double gPeriod;

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

    /**
     * Restart measurement and return previous time.
     */
    double Restart();
};

} // namespace Common
} // namespace NFE
