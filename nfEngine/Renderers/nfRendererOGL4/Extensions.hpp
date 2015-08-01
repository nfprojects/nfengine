/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of OpenGL extensions
 */

#pragma once

// TODO The implementation defined here is WINDOWS only.
//      At some point, some extensions will become platform-specific.
#ifdef WIN32
// Windows has its needs - OGL calls extracted with wglGetProcAddress must have __stdcall keyword
#define STDCALL __stdcall
#else
#define STDCALL
#endif

/////////////////////////
/// Additional macros ///
/////////////////////////

/// OpenGL 2.0
#define GL_VERTEX_SHADER                    0x8B31
#define GL_FRAGMENT_SHADER                  0x8B30
#define GL_LINK_STATUS                      0x8B82
#define GL_VALIDATE_STATUS                  0x8B83
#define GL_INFO_LOG_LENGTH                  0x8B84

/// OpenGL 3.2
#define GL_GEOMETRY_SHADER                  0x8DD9

/// ARB_tesselation_shader
#define GL_TESS_EVALUATION_SHADER           0x8E87
#define GL_TESS_CONTROL_SHADER              0x8E88

/// ARB_separate_shader_objects
#define GL_VERTEX_SHADER_BIT                0x00000001
#define GL_FRAGMENT_SHADER_BIT              0x00000002
#define GL_GEOMETRY_SHADER_BIT              0x00000004
#define GL_TESS_CONTROL_SHADER_BIT          0x00000008
#define GL_TESS_EVALUATE_SHADER_BIT         0x00000010
#define GL_ALL_SHADER_BITS                  0xFFFFFFFF
#define GL_PROGRAM_SEPARABLE                0x8258
#define GL_ACTIVE_PROGRAM                   0x8259
#define GL_PROGRAM_PIPELINE_BINDING         0x825A


/////////////////////////////////
/// Function pointer typedefs ///
/////////////////////////////////

/// WGL_EXT_swap_control
typedef BOOL (STDCALL *wglSwapIntervalEXTType)(int interval);
typedef int (STDCALL *wglGetSwapIntervalEXTType)(void);

/// OpenGL 2.0
typedef void (STDCALL *glGetProgramivType)(GLuint program, GLenum pname, GLint *params);
typedef void (STDCALL *glGetProgramInfoLogType)(GLuint program, GLsizei bufSize, GLsizei* length, char* infoLog);
typedef void (STDCALL *glDeleteProgramType)(GLuint program);

/// ARB_separate_shader_objects
typedef void (STDCALL *glUseProgramStagesType)(GLuint pipeline, GLbitfield stages, GLuint program);
typedef void (STDCALL *glActiveShaderProgramType)(GLuint pipeline, GLuint program);
typedef GLuint (STDCALL *glCreateShaderProgramvType)(GLenum type, GLsizei count, const char** strings);
typedef void (STDCALL *glBindProgramPipelineType)(GLuint pipeline);
typedef void (STDCALL *glDeleteProgramPipelinesType)(GLsizei n, const GLuint* pipelines);
typedef void (STDCALL *glGenProgramPipelinesType)(GLsizei n, GLuint* pipelines);
typedef void (STDCALL *glProgramParameteriType)(GLuint program, GLenum pname, GLint value);


/////////////////////////
/// Function pointers ///
/////////////////////////

/// WGL_EXT_swap_control
extern wglSwapIntervalEXTType wglSwapIntervalEXT;
extern wglGetSwapIntervalEXTType wglGetSwapIntervalEXT;

/// OpenGL 2.0
extern glGetProgramivType glGetProgramiv;
extern glGetProgramInfoLogType glGetProgramInfoLog;
extern glDeleteProgramType glDeleteProgram;

/// ARB_separate_shader_objects
extern glUseProgramStagesType glUseProgramStages;
extern glActiveShaderProgramType glActiveShaderProgram;
extern glCreateShaderProgramvType glCreateShaderProgramv;
extern glBindProgramPipelineType glBindProgramPipeline;
extern glDeleteProgramPipelinesType glDeleteProgramPipelines;
extern glGenProgramPipelinesType glGenProgramPipelines;
extern glProgramParameteriType glProgramParameteri;


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
