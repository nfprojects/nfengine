/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Utilities for Console Interface
 */

#include "../nfCommon.hpp"

#if defined(WIN32)
#include "Windows/Console.hpp"
#elif defined(__LINUX__) | defined(__linux__)
#include "Linux/Console.hpp"
#else
#error "Target system not supported!"
#endif // ifdef WIN32 elif __LINUX__  | __linux__
