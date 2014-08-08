/**
    NFEngine project

    \file   Timer.cpp
    \brief  High resoulution timer class definition.
*/

#include "stdafx.h"
#include "Timer.h"

namespace NFE {
namespace Common {

Timer::Timer()
{

}

void Timer::Start()
{
    mStart = Clock::now();
}

double Timer::Stop()
{
    auto nanosec = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - mStart).count();
    return static_cast<double>(nanosec) * 1.0e-9;
}

} // namespace Common
} // namespace NFE