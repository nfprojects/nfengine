/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Utilities for Console Interface
 */

#include "../nfCommon.hpp"

#if defined(NFE_PLATFORM_WINDOWS)
#include "Windows/Console.hpp"
#elif defined(NFE_PLATFORM_LINUX)
#include "Linux/Console.hpp"
#else
#error Invalid platform
#endif
