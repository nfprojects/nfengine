#pragma once

// enable memory allocation tracking (Windows only)
#if defined(NFE_PLATFORM_WINDOWS) && defined(NFE_CONFIGURATION_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // defined(NFE_PLATFORM_WINDOWS) && defined(NFE_CONFIGURATION_DEBUG)

#if defined(NFE_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // defined(NFE_PLATFORM_WINDOWS)

#include <memory>
#include <vector>
#include <string>

#include "gtest/gtest.h"
