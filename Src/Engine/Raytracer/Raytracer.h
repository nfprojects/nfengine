#pragma once

#include "../Common/nfCommon.hpp"
#include "Config.h"
#include "ForwardDeclarations.hpp"

// DLL import / export macro
#ifdef WIN32

#ifdef NFE_RAYTRACER_EXPORTS
#define NFE_RAYTRACER_API __declspec(dllexport)
#else // RAYLIB_EXPORTS
#define NFE_RAYTRACER_API __declspec(dllimport)
#endif // RAYLIB_EXPORTS

#else // WIN32

#define NFE_RAYTRACER_API __attribute__((visibility("default")))

#endif // WIN32
