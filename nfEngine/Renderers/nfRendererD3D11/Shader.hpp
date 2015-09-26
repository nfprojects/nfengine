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

/**
 * Shader resource binding description
 */
struct ShaderResBindings
{
    std::map<std::string, int> textures;
    std::map<std::string, int> cbuffers;
    std::map<std::string, int> samplers;
};

class Shader : public IShader
{
    friend class ShaderProgram;
    friend class CommandBuffer;

    ShaderType mType;
    D3DPtr<ID3DBlob> mBytecode;
    ShaderResBindings mResBindings;

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
    bool Disassemble(bool html, std::string& output);
};


struct ShaderProgramResBinding
{
    std::string name;
    int vertexShaderSlot;
    int geometryShaderSlot;
    int hullShaderSlot;
    int domainShaderSlot;
    int pixelShaderSlot;

    ShaderProgramResBinding()
        : vertexShaderSlot(-1)
        , geometryShaderSlot(-1)
        , hullShaderSlot(-1)
        , domainShaderSlot(-1)
        , pixelShaderSlot(-1)
    { }
};

/**
 * Shader program resource binding description
 */
struct ShaderProgramResBindings
{
    std::vector<ShaderProgramResBinding> textures;
    std::vector<ShaderProgramResBinding> cbuffers;
    std::vector<ShaderProgramResBinding> samplers;
};

class ShaderProgram : public IShaderProgram
{
    friend class CommandBuffer;

    ShaderProgramDesc mDesc;
    ShaderProgramResBindings mResBinding;

public:
    ShaderProgram(const ShaderProgramDesc& desc);
    const ShaderProgramDesc& GetDesc() const;

    int GetTextureSlotByName(const char* textureName);
    int GetCBufferSlotByName(const char* cbufferName);
    int GetSamplerSlotByName(const char* samplerName);
};

} // namespace Renderer
} // namespace NFE
