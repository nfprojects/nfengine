/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  AsyncQueueManager utility declarations.
 */

#pragma once

#if defined(WIN32)
#include "Windows/AsyncQueueManager.hpp"
#elif defined(__LINUX__) | defined(__linux__)
#include "Linux/AsyncQueueManager.hpp"
#endif