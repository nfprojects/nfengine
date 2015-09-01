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
    GLuint mShader;

public:
    Shader();
    ~Shader();
    bool Init(const ShaderDesc& desc);
    GLuint GetShader();

    bool GetIODesc(ShaderIODesc& result);
    bool Disassemble(bool html, std::string& output);
};

} // namespace Renderer
} // namespace NFE
