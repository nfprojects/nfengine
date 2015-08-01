/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of OpenGL extensions used in functions
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "Extensions.hpp"

/// WGL_EXT_swap_control
wglSwapIntervalEXTType wglSwapIntervalEXT = nullptr;
wglGetSwapIntervalEXTType wglGetSwapIntervalEXT = nullptr;

/// OpenGL 2.0
glGetProgramivType glGetProgramiv = nullptr;
glGetProgramInfoLogType glGetProgramInfoLog = nullptr;
glDeleteProgramType glDeleteProgram = nullptr;

/// ARB_separate_shader_objects
glUseProgramStagesType glUseProgramStages = nullptr;
glActiveShaderProgramType glActiveShaderProgram = nullptr;
glCreateShaderProgramvType glCreateShaderProgramv = nullptr;
glBindProgramPipelineType glBindProgramPipeline = nullptr;
glDeleteProgramPipelinesType glDeleteProgramPipelines = nullptr;
glGenProgramPipelinesType glGenProgramPipelines = nullptr;
glProgramParameteriType glProgramParameteri = nullptr;

/**
 * Get an OpenGL extension using wglGetProcAddress.
 *
 * wglGetProcAddress returns a nullptr when the extension is not available. When such situation
 * occurs, appropriate information is logged and allExtensionsAvailable is set to false.
 */
#define NFGL_GET_EXTENSION(x)                                                               \
do {                                                                                        \
    x = reinterpret_cast<x##Type>(wglGetProcAddress(#x));                                   \
    if (!x)                                                                                 \
    {                                                                                       \
        LOG_ERROR("Unable to retrieve " #x " extension function: %lu.", GetLastError());    \
        allExtensionsAvailable = false;                                                     \
    }                                                                                       \
} while(0)

namespace NFE {
namespace Renderer {

bool nfglExtensionsInit()
{
    bool allExtensionsAvailable = true;

    /// WGL_EXT_swap_control
    NFGL_GET_EXTENSION(wglSwapIntervalEXT);
    NFGL_GET_EXTENSION(wglGetSwapIntervalEXT);

    /// OpenGL 2.0
    NFGL_GET_EXTENSION(glGetProgramiv);
    NFGL_GET_EXTENSION(glGetProgramInfoLog);
    NFGL_GET_EXTENSION(glDeleteProgram);

    /// ARB_separate_shader_objects
    NFGL_GET_EXTENSION(glUseProgramStages);
    NFGL_GET_EXTENSION(glActiveShaderProgram);
    NFGL_GET_EXTENSION(glCreateShaderProgramv);
    NFGL_GET_EXTENSION(glBindProgramPipeline);
    NFGL_GET_EXTENSION(glDeleteProgramPipelines);
    NFGL_GET_EXTENSION(glGenProgramPipelines);
    NFGL_GET_EXTENSION(glProgramParameteri);

    return allExtensionsAvailable;
}

} // namespace Renderer
} // namespace NFE
