/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan Shader declarations
 */

#pragma once

#include "../RendererInterface/Shader.hpp"
#include "Defines.hpp"

namespace NFE {
namespace Renderer {

class Shader : public IShader
{
    friend class CommandBuffer;
    friend class ShaderProgram;

    ShaderType mType;
    std::unique_ptr<glslang::TShader> mShaderGlslang;
    std::unique_ptr<glslang::TProgram> mProgramGlslang;
    std::vector<uint32> mShaderSpv;
    VkShaderModule mShader;

    bool GetIODesc();

public:
    Shader();
    ~Shader();
    bool Init(const ShaderDesc& desc);

    bool Disassemble(bool html, std::string& output);
};

} // namespace Renderer
} // namespace NFE
