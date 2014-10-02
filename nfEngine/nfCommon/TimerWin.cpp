/**
 *  @file   TimerWin.cpp
 *  @brief  High resolution timer class definition for Windows system
 */

#include "stdafx.h"
#include "TimerWin.hpp"

namespace NFE {
namespace Common {

Timer::Timer()
{
    QueryPerformanceFrequency(&mFreq);
    mStart.QuadPart = 0;
    mStop.QuadPart = 0;
}

void Timer::Start()
{
    QueryPerformanceCounter(&mStart);
}

double Timer::Stop()
{
    QueryPerformanceCounter(&mStop);

    return static_cast<double>(mStop.QuadPart - mStart.QuadPart) /
        static_cast<double>(mFreq.QuadPart);
}

} // namespace Common
} // namespace NFE
