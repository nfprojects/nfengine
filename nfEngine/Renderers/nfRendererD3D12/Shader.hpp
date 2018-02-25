/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 render's shader
 */

#pragma once

#include "../RendererInterface/Shader.hpp"
#include "Common.hpp"
#include "../../nfCommon/Containers/HashMap.hpp"


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

    friend class CommandRecorder;

    ShaderType mType;
    D3DPtr<ID3DBlob> mBytecode;
    Common::HashMap<Common::String, ResBinding> mResBindings;

    bool GetIODesc();

public:
    Shader();
    bool Init(const ShaderDesc& desc);
    ID3DBlob* GetBytecode() const;
    D3D12_SHADER_BYTECODE GetD3D12Bytecode() const;

    bool Disassemble(bool html, Common::String& output) override;
    int GetResourceSlotByName(const char* name) override;
};

using InternalShaderPtr = Common::SharedPtr<Shader>;

} // namespace Renderer
} // namespace NFE
