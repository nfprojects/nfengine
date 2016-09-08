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
        ID3D11ComputeShader* mCS;

        ID3D11DeviceChild* mGeneric;
    };

    bool GetIODesc();

public:
    Shader();
    ~Shader();
    bool Init(const ShaderDesc& desc);
    void* GetShaderObject() const;
    ID3DBlob* GetBytecode() const;

    bool Disassemble(bool html, std::string& output) override;
    int GetResourceSlotByName(const char* name) override;
};

} // namespace Renderer
} // namespace NFE
