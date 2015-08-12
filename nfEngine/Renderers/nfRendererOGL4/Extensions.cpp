/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of OpenGL extensions used in functions
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "Extensions.hpp"

#include "GetExtension.hpp"


namespace NFE {
namespace Renderer {

/// OpenGL 1.5
PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;

/// OpenGL 2.0
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = nullptr;

/// ARB_separate_shader_objects
PFNGLUSEPROGRAMSTAGESPROC glUseProgramStages = nullptr;
PFNGLACTIVESHADERPROGRAMPROC glActiveShaderProgram = nullptr;
PFNGLCREATESHADERPROGRAMVPROC glCreateShaderProgramv = nullptr;
PFNGLBINDPROGRAMPIPELINEPROC glBindProgramPipeline = nullptr;
PFNGLDELETEPROGRAMPIPELINESPROC glDeleteProgramPipelines = nullptr;
PFNGLGENPROGRAMPIPELINESPROC glGenProgramPipelines = nullptr;
PFNGLPROGRAMPARAMETERIPROC glProgramParameteri = nullptr;


bool nfglExtensionsInit()
{
    bool allExtensionsAvailable = true;

#ifdef WIN32
    if (!nfwglExtensionsInit())
        LOG_WARNING("WGL extensions failed to init, some WGL APIs might be unavailable.");
#elif defined(__linux__) | defined(__LINUX__)
    if (!nfglxExtensionsInit())
        LOG_WARNING("GLX extensions failed to init, some GLX APIs might be unavailable.");
#else
#error "Target platform not supported."
#endif

    /// OpenGL 1.5
    NFGL_GET_EXTENSION(PFNGLGENBUFFERSPROC, glGenBuffers);
    NFGL_GET_EXTENSION(PFNGLBINDBUFFERPROC, glBindBuffer);
    NFGL_GET_EXTENSION(PFNGLBUFFERDATAPROC, glBufferData);
    NFGL_GET_EXTENSION(PFNGLDELETEBUFFERSPROC, glDeleteBuffers);

    /// OpenGL 2.0
    NFGL_GET_EXTENSION(PFNGLGETPROGRAMIVPROC, glGetProgramiv);
    NFGL_GET_EXTENSION(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog);
    NFGL_GET_EXTENSION(PFNGLDELETEPROGRAMPROC, glDeleteProgram);
    NFGL_GET_EXTENSION(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer);
    NFGL_GET_EXTENSION(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray);
    NFGL_GET_EXTENSION(PFNGLDISABLEVERTEXATTRIBARRAYPROC, glDisableVertexAttribArray);

    /// ARB_separate_shader_objects
    NFGL_GET_EXTENSION(PFNGLUSEPROGRAMSTAGESPROC, glUseProgramStages);
    NFGL_GET_EXTENSION(PFNGLACTIVESHADERPROGRAMPROC, glActiveShaderProgram);
    NFGL_GET_EXTENSION(PFNGLCREATESHADERPROGRAMVPROC, glCreateShaderProgramv);
    NFGL_GET_EXTENSION(PFNGLBINDPROGRAMPIPELINEPROC, glBindProgramPipeline);
    NFGL_GET_EXTENSION(PFNGLDELETEPROGRAMPIPELINESPROC, glDeleteProgramPipelines);
    NFGL_GET_EXTENSION(PFNGLGENPROGRAMPIPELINESPROC, glGenProgramPipelines);
    NFGL_GET_EXTENSION(PFNGLPROGRAMPARAMETERIPROC, glProgramParameteri);

    return allExtensionsAvailable;
}

} // namespace Renderer
} // namespace NFE
