#pragma once

#include "../Common/nfCommon.hpp"

// DLL import / export macro
#ifdef WIN32

#ifdef NFE_CORE_EXPORTS
#define RAYLIB_API __declspec(dllexport)
#else // RAYLIB_EXPORTS
#define RAYLIB_API __declspec(dllimport)
#endif // RAYLIB_EXPORTS

#else // WIN32

#define RAYLIB_API __attribute__((visibility("default")))

#endif // WIN32
