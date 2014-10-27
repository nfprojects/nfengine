/**
 *  @file   TimerWin.cpp
 *  @brief  High resolution timer class definition for Windows system
 */

#include "stdafx.hpp"
#include "TimerWin.hpp"


namespace {
static double GetCounterPeriod()
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return 1.0 / static_cast<double>(freq.QuadPart);
}

static const double mPeriod = GetCounterPeriod();
} // namespace


namespace NFE {
namespace Common {

Timer::Timer()
{
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
