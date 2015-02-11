/**
 * @file   stdafx.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Precompiled header.
 */

#pragma once

#ifdef WIN32
#include "targetver.hpp"

// TODO: remove platform-dependent libraries!
//       or move them to some #ifdef...#endif blocks
#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>
#endif

/// C lib
#include <stdlib.h>
#include <time.h>

/// STL
#include <vector>
