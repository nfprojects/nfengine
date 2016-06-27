/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Common defines for Vulkan Renderer
 */

#pragma once

// DLL import / export macro
#ifdef WIN32
#ifdef RENDERERVK_EXPORTS
#define RENDERER_API __declspec(dllexport)
#else // RENDERERVK_EXPORTS
#define RENDERER_API __declspec(dllimport)
#endif // RENDERERVK_EXPORTS
#else // WIN32
#define RENDERER_API __attribute__((visibility("default")))
#endif // WIN32

// macro for unused parameters to supress warnings
#define UNUSED(x) (void)(x)

#include "vulkan/vulkan.h"

// LKTODO below code needs recreation in Vulkan (it will probably be veeeery useful...)
#if 0

#ifdef _DEBUG

#include <GL/glu.h> // only for gluErrorString()

/**
 * Macro for error checking in OpenGL.
 *
 * Ideally release code should not contain this macro (it is for debug purposes only).
 */
#ifndef GL_GET_ERRORS
#define GL_GET_ERRORS()                                                         \
do                                                                              \
{                                                                               \
    GLenum err;                                                                 \
    while ((err = glGetError()) != GL_NO_ERROR)                                 \
    {                                                                           \
        const char* str = reinterpret_cast<const char*>(gluErrorString(err));   \
        LOG_ERROR("OpenGL error %d: ", err, str);                               \
    }                                                                           \
    LOG_DEBUG("==== OpenGL Error queue empty ====");                            \
} while(0)
#endif // GL_GET_ERRORS

#else // _DEBUG

#ifndef GL_GET_ERRORS
#define GL_GET_ERRORS()
#endif // GL_GET_ERRORS

#endif // _DEBUG

#endif // 0

#include "Extensions.hpp"
#include "Logger.hpp"
