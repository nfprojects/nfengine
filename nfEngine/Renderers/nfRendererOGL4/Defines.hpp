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

#include "gl/gl.h"
#include "gl/glu.h"

#include "Extensions.hpp"

#include "Logger.hpp"
