/**
 * @file
 * @author Lookey (costyrra.xl@gmail.com)
 * @brief  High resolution timer class definition for Windows system
 */

#include "PCH.hpp"
#include "../Timer.hpp"


namespace {
static double GetCounterPeriod()
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return 1.0 / static_cast<double>(freq.QuadPart);
}
} // namespace


namespace NFE {
namespace Common {

const double Timer::gPeriod = GetCounterPeriod();

Timer::Timer()
{
    Start();
}

void Timer::Start()
{
    QueryPerformanceCounter(&mStart);
}

double Timer::Restart()
{
    LARGE_INTEGER previousTime = mStart;
    QueryPerformanceCounter(&mStart);

    return static_cast<double>(mStart.QuadPart - previousTime.QuadPart) * gPeriod;
}

double Timer::Stop()
{
    LARGE_INTEGER stop;
    QueryPerformanceCounter(&stop);

    return static_cast<double>(stop.QuadPart - mStart.QuadPart) * gPeriod;
}

} // namespace Common
} // namespace NFE
