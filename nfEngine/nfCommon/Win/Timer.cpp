/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  High resolution timer class definition for Windows system
 */

#include "../PCH.hpp"
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

Timer::Timer()
{
    static double gPeriod = GetCounterPeriod();
    mPeriod = gPeriod;

    mStart.QuadPart = 0;
}

void Timer::Start()
{
    QueryPerformanceCounter(&mStart);
}

double Timer::Stop()
{
    LARGE_INTEGER stop;
    QueryPerformanceCounter(&stop);

    return static_cast<double>(stop.QuadPart - mStart.QuadPart) * mPeriod;
}

} // namespace Common
} // namespace NFE
