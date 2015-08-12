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
PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLDETACHSHADERPROC glDetachShader = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;
PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = nullptr;

/// OpenGL 3.0
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;

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
    NFGL_GET_EXTENSION(PFNGLCREATESHADERPROC, glCreateShader);
    NFGL_GET_EXTENSION(PFNGLGETSHADERIVPROC, glGetShaderiv);
    NFGL_GET_EXTENSION(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog);
    NFGL_GET_EXTENSION(PFNGLSHADERSOURCEPROC, glShaderSource);
    NFGL_GET_EXTENSION(PFNGLCOMPILESHADERPROC, glCompileShader);
    NFGL_GET_EXTENSION(PFNGLATTACHSHADERPROC, glAttachShader);
    NFGL_GET_EXTENSION(PFNGLDETACHSHADERPROC, glDetachShader);
    NFGL_GET_EXTENSION(PFNGLDELETESHADERPROC, glDeleteShader);
    NFGL_GET_EXTENSION(PFNGLCREATEPROGRAMPROC, glCreateProgram);
    NFGL_GET_EXTENSION(PFNGLGETPROGRAMIVPROC, glGetProgramiv);
    NFGL_GET_EXTENSION(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog);
    NFGL_GET_EXTENSION(PFNGLLINKPROGRAMPROC, glLinkProgram);
    NFGL_GET_EXTENSION(PFNGLUSEPROGRAMPROC, glUseProgram);
    NFGL_GET_EXTENSION(PFNGLDELETEPROGRAMPROC, glDeleteProgram);
    NFGL_GET_EXTENSION(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer);
    NFGL_GET_EXTENSION(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray);
    NFGL_GET_EXTENSION(PFNGLDISABLEVERTEXATTRIBARRAYPROC, glDisableVertexAttribArray);

    /// OpenGL 3.0
    NFGL_GET_EXTENSION(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays);
    NFGL_GET_EXTENSION(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray);
    NFGL_GET_EXTENSION(PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays);

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
