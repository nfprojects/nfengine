/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of OpenGL extensions
 */

#pragma once

#include "GL/glext.h"

#ifdef WIN32
#include "Win/wglExtensions.hpp"
#elif defined(__linux__) | defined(__LINUX__)
#include "Linux/glXExtensions.hpp"
#else
#error "Target platform not supported."
#endif


/// OpenGL 2.0
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;

/// ARB_separate_shader_objects
extern PFNGLUSEPROGRAMSTAGESPROC glUseProgramStages;
extern PFNGLACTIVESHADERPROGRAMPROC glActiveShaderProgram;
extern PFNGLCREATESHADERPROGRAMVPROC glCreateShaderProgramv;
extern PFNGLBINDPROGRAMPIPELINEPROC glBindProgramPipeline;
extern PFNGLDELETEPROGRAMPIPELINESPROC glDeleteProgramPipelines;
extern PFNGLGENPROGRAMPIPELINESPROC glGenProgramPipelines;
extern PFNGLPROGRAMPARAMETERIPROC glProgramParameteri;


namespace NFE {
namespace Renderer {

/**
 * Initializes OpenGL extensions used by nfRendererOGL4.
 *
 * The function performs all the initialization work needed to enable extensions used by OpenGL
 * implementation of nfRenderer. Failing to get any of the extensions will result in an error.
 *
 * @return True if all extensions were successfully acquired, false otherwise.
 */
bool nfglExtensionsInit();

} // namespace Renderer
} // namespace NFE
