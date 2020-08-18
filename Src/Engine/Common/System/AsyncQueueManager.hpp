/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  AsyncQueueManager utility declarations.
 */

#pragma once

#if defined(NFE_PLATFORM_WINDOWS)
#include "Windows/AsyncQueueManager.hpp"
#elif defined(NFE_PLATFORM_LINUX)
#include "Linux/AsyncQueueManager.hpp"
#else
#error Invalid platform
#endif