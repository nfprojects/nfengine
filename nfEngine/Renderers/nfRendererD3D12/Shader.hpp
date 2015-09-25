/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 render's shader
 */

#pragma once

#include "../RendererInterface/Shader.hpp"
#include "Common.hpp"

namespace NFE {
namespace Renderer {

class Shader : public IShader
{
    friend class CommandBuffer;

    ShaderType mType;
    D3DPtr<ID3DBlob> mBytecode;

public:
    Shader();
    bool Init(const ShaderDesc& desc);
    ID3DBlob* GetBytecode() const;

    bool GetIODesc(ShaderIODesc& result);
    bool Disassemble(bool html, std::string& output);
};

class ShaderProgram : public IShaderProgram
{
    ShaderProgramDesc mDesc;
public:
    ShaderProgram(const ShaderProgramDesc& desc);
    const ShaderProgramDesc& GetDesc() const;
};

} // namespace Renderer
} // namespace NFE
