/**
    @file   Timer.hpp
    @brief  High resolution timer class declaration.
*/

#pragma once

#include "nfCommon.h"

#if defined(WIN32)
#include "TimerWin.hpp"
#elif defined(__LINUX__) | defined(__linux__)
#include "TimerLinux.hpp"
#else
#error "Target system not supported!"
#endif // ifdef WIN32 elif __LINUX__  | __linux__
