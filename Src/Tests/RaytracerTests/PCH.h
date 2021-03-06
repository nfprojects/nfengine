#pragma once

#if defined(_DEBUG) && defined(WIN32)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _DEBUG

#include <sstream>
#include <array>
#include <string>
#include <vector>
#include <memory>
#include <limits>
#include <functional>

#include "gtest/gtest.h"
