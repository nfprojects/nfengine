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
    struct ResBinding
    {
        ShaderResourceType type;
        int slot;
        ResBinding() : type(ShaderResourceType::Unknown), slot(0) { }
    };

    friend class ShaderProgram;
    friend class CommandBuffer;

    ShaderType mType;
    D3DPtr<ID3DBlob> mBytecode;
    std::map<std::string, ResBinding> mResBindings;

    bool GetIODesc();

public:
    Shader();
    bool Init(const ShaderDesc& desc);
    ID3DBlob* GetBytecode() const;
    D3D12_SHADER_BYTECODE GetD3D12Bytecode() const;

    bool Disassemble(bool html, std::string& output);
};

class ShaderProgram : public IShaderProgram
{
    ShaderProgramDesc mDesc;

public:
    ShaderProgram(const ShaderProgramDesc& desc);
    ~ShaderProgram();

    const ShaderProgramDesc& GetDesc() const;
    int GetResourceSlotByName(const char* name) override;
};

extern const int SHADER_TYPE_BIT_OFFSET;
extern const int SHADER_RES_TYPE_BIT_OFFSET;
extern const int SHADER_RES_SLOT_MASK;

} // namespace Renderer
} // namespace NFE
