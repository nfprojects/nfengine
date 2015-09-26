/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   OGL4 Shader declarations
 */

#include "../RendererInterface/Shader.hpp"
#include "Defines.hpp"

namespace NFE {
namespace Renderer {

class ShaderProgram: public IShaderProgram
{
    friend class CommandBuffer;

    GLuint mProgramPipeline;

public:
    ShaderProgram();
    ~ShaderProgram();
    bool Init(const ShaderProgramDesc& desc);
    int GetTextureSlotByName(const char* textureName);
    int GetCBufferSlotByName(const char* cbufferName);
    int GetSamplerSlotByName(const char* samplerName);
};

class Shader: public IShader
{
    friend class CommandBuffer;
    friend class ShaderProgram;

    ShaderType mType;

    /**
     * The naming convention is a bit different in OpenGL with SSO, comparing to D3D.
     *
     * To implement SSO, OpenGL allows creation of a ShaderProgram with only one Shader attached.
     * Thus, D3D's Shader is OGL's ShaderProgram, and the equivalent of D3D's ShaderProgram is a
     * ProgramPipeline, newly added by SSO.
     *
     * Refer to ARB_separate_shader_objects description on opengl.org for more info about SSO.
     */
    GLuint mShaderProgram;

public:
    Shader();
    ~Shader();
    bool Init(const ShaderDesc& desc);
    GLuint GetShaderProgram();

    bool Disassemble(bool html, std::string& output);
};

} // namespace Renderer
} // namespace NFE
