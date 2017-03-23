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

    GLuint mProgram;

public:
    ShaderProgram();
    ~ShaderProgram();
    bool Init(const ShaderProgramDesc& desc);
};

class Shader: public IShader
{
    friend class CommandBuffer;
    friend class ShaderProgram;

    ShaderType mType;
    GLuint mShader;        //< Old-style shader, only compiled code
    GLuint mShaderProgram; //< SSO-style shader program, compiled and linked

public:
    Shader();
    ~Shader();
    bool Init(const ShaderDesc& desc);

    bool GetIODesc(ShaderIODesc& result);
    bool Disassemble(bool html, String& output);
};

} // namespace Renderer
} // namespace NFE
