/**
 *  @file
 *  @brief  High resolution timer class definition for Linux systems
 */

#include "PCH.hpp"
#include "../Timer.hpp"


namespace NFE {
namespace Common {

Timer::Timer()
{
    mStart.tv_sec = 0;
    mStart.tv_nsec = 0;
}

void Timer::Start()
{
    clock_gettime(CLOCK_MONOTONIC, &mStart);
}

double Timer::Stop()
{
    struct timespec mStop;
    clock_gettime(CLOCK_MONOTONIC, &mStop);

    // negative time difference should not occur when using CLOCK_MONOTONIC
    time_t sec_result = (mStop.tv_sec - mStart.tv_sec) * 1000000000;
    double result = (double)(sec_result + (mStop.tv_nsec - mStart.tv_nsec)) * 1e-9;
    return result;
}

} // namespace Common
} // namespace NFE

