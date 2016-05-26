/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Precompiled header. Not modified headers (STL, external libraries' APIs, etc.)
 *         should be put here.
 */

#pragma once

//suppress trivial warnings
#pragma warning (disable: 4251)
#pragma warning (disable: 4100)
#pragma warning (disable: 4127)
#pragma warning (disable: 4324)
#pragma warning (disable: 4099)

// VS memory leak detector
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

//WinAPI
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <WindowsX.h>

// C lib
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <setjmp.h>
#include <time.h>
#include <sys/stat.h>

// STL lib
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <queue>
#include <memory>
#include <sstream>
#include <tuple>

//Bullet physics Engine
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

//FreeType library
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"
#include "freetype/ftoutln.h"
#include "freetype/fttrigon.h"

/// RapidXML library
#include "rapidxml.hpp"
#include "rapidxml_iterators.hpp"
#include "rapidxml_print.hpp"

#ifdef GetWindowFont
#undef GetWindowFont  // ImGui workaround - GetWindowFont is both WinAPI macro and ImGui function
#endif
#include "imgui.h"
