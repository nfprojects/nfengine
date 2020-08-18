#pragma once

#if defined(NFE_CONFIGURATION_DEBUG) && defined(NFE_PLATFORM_WINDOWS)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // NFE_CONFIGURATION_DEBUG

#include <sstream>
#include <memory>
#include <inttypes.h>
#include <stddef.h>
#include <float.h>
