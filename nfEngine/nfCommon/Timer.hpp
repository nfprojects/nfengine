/**
 * @file   Timer.hpp
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  High resolution timer class declaration.
 */

#pragma once

#include "nfCommon.hpp"

#if defined(WIN32)
#include "Win/Timer.hpp"
#elif defined(__LINUX__) | defined(__linux__)
#include "Linux/Timer.hpp"
#else
#error "Target system not supported!"
#endif // ifdef WIN32 elif __LINUX__  | __linux__
