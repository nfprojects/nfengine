/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Precompiled header
 */

#pragma once

 // enable memory allocation tracking (Windows only)
#if defined(WIN32) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // defined(WIN32) && defined(_DEBUG)

#include <iostream>
#include <memory>
#include <fstream>
#include <vector>