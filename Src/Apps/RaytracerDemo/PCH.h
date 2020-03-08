#pragma once

#if defined(_DEBUG) && defined(WIN32)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _DEBUG

#include <sstream>
#include <memory>
#include <inttypes.h>
#include <stddef.h>
#include <float.h>
