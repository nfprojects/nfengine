/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 11 render's shader
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

    union
    {
        ID3D11VertexShader* mVS;
        ID3D11GeometryShader* mGS;
        ID3D11HullShader* mHS;
        ID3D11DomainShader* mDS;
        ID3D11PixelShader* mPS;
        void* mGeneric;
    };

public:
    Shader();
    ~Shader();
    bool Init(const ShaderDesc& desc);
    void* GetShaderObject() const;
    ID3DBlob* GetBytecode() const;

    bool GetIODesc(ShaderIODesc& result) override;
    bool Disassemble(bool html, std::string& output) override;
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
