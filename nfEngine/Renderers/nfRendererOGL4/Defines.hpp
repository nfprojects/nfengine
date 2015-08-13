/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Common defines for OpenGL Renderer
 */

#pragma once

// DLL import / export macro
#ifdef WIN32
#ifdef RENDEREROGL4_EXPORTS
#define RENDERER_API __declspec(dllexport)
#else // RENDEREROGL4_EXPORTS
#define RENDERER_API __declspec(dllimport)
#endif // RENDEREROGL4_EXPORTS
#else // WIN32
#define RENDERER_API __attribute__((visibility("default")))
#endif // WIN32

// macro for unused parameters to supress warnings
#define UNUSED(x) (void)(x)

#include "GL/gl.h"
#include "GL/glu.h"

#if defined(__linux__) | defined(__LINUX__)
#include "GL/glx.h"
#endif

#include "Extensions.hpp"

#include "Logger.hpp"
