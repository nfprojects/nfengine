/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 11 render's shader
 */

#pragma once

#include "../RendererInterface/Shader.hpp"
#include "Common.hpp"
#include <map>

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

    union
    {
        ID3D11VertexShader* mVS;
        ID3D11GeometryShader* mGS;
        ID3D11HullShader* mHS;
        ID3D11DomainShader* mDS;
        ID3D11PixelShader* mPS;
        void* mGeneric;
    };

    bool GetIODesc();

public:
    Shader();
    ~Shader();
    bool Init(const ShaderDesc& desc);
    void* GetShaderObject() const;
    ID3DBlob* GetBytecode() const;

    bool Disassemble(bool html, std::string& output) override;
};

class ShaderProgram : public IShaderProgram
{
    ShaderProgramDesc mDesc;
public:
    ShaderProgram(const ShaderProgramDesc& desc);
    const ShaderProgramDesc& GetDesc() const;
    int GetResourceSlotByName(const char* name);
};

extern const int SHADER_TYPE_BIT_OFFSET;
extern const int SHADER_RES_TYPE_BIT_OFFSET;
extern const int SHADER_RES_SLOT_MASK;

} // namespace Renderer
} // namespace NFE
