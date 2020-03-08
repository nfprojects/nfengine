/**
* @file
* @author  Witek902 (witek902@gmail.com)
* @brief   D3D12 implementation of renderer's shader
*/

#include "PCH.hpp"
#include "Shader.hpp"
#include "RendererD3D12.hpp"

#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Common/FileSystem/File.hpp"


namespace NFE {
namespace Renderer {

Shader::Shader()
    : mType(ShaderType::Unknown)
{}

bool Shader::Init(const ShaderDesc& desc)
{
    mType = desc.type;

    HRESULT hr;
    Common::DynArray<char> str;
    size_t shaderSize = 0;
    const char* code = nullptr;

    if (desc.code == nullptr)
    {
        if (desc.path == nullptr)
        {
            NFE_LOG_ERROR("Shader code or path must be suplied");
            return false;
        }

        using namespace Common;
        File file(Common::StringView(desc.path), AccessMode::Read);
        const uint32 size = static_cast<uint32>(file.GetSize());
        str.Resize(size + 1);
        shaderSize = size;

        if (file.Read(str.Data(), shaderSize) != shaderSize)
            return false;
        str[size] = '\0';
        code = str.Data();
    }
    else
    {
        code = desc.code;
        shaderSize = strlen(code);
    }

    DWORD flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
#ifdef _DEBUG //shaders debugging
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

    // TODO: max profile level detection
    const char* profileName;
    switch (mType)
    {
    case ShaderType::Vertex:
        profileName = "vs_5_0";
        break;
    case ShaderType::Geometry:
        profileName = "gs_5_0";
        break;
    case ShaderType::Hull:
        profileName = "hs_5_0";
        break;
    case ShaderType::Domain:
        profileName = "ds_5_0";
        break;
    case ShaderType::Pixel:
        profileName = "ps_5_0";
        break;
    case ShaderType::Compute:
        profileName = "cs_5_0";
        break;
    default:
        NFE_LOG_ERROR("Invalid shader type");
        return false;
    }

    Common::String macrosStr;
    Common::UniquePtr<D3D_SHADER_MACRO[]> d3dMacros;
    if (desc.macrosNum > 0)
    {
        d3dMacros.Reset(new D3D_SHADER_MACRO[desc.macrosNum + 1]);
        for (size_t i = 0; i < desc.macrosNum; ++i)
        {
            d3dMacros[i].Name = desc.macros[i].name;
            d3dMacros[i].Definition = desc.macros[i].value;

            macrosStr += Common::String('\'') + d3dMacros[i].Name + "' = '" + desc.macros[i].value + '\'';
            if (i < desc.macrosNum - 1)
                macrosStr += ", ";
        }
        d3dMacros[desc.macrosNum].Name = nullptr;
        d3dMacros[desc.macrosNum].Definition = nullptr;
    }

    NFE_LOG_INFO("Compiling shader '%s' with macros: [%s]...", desc.path, macrosStr.Str());

    ID3DBlob* errorsBuffer = nullptr;
    hr = D3DCompile(code, shaderSize, desc.path, d3dMacros.Get(),
                    D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", profileName, flags, 0,
                    mBytecode.GetPtr(), &errorsBuffer);

    if (errorsBuffer)
    {
        NFE_LOG_ERROR("Shader '%s' compilation output:\n%s", desc.path, (char*)errorsBuffer->GetBufferPointer());
        errorsBuffer->Release();
    }

    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Compilation of shader '%s' failed", desc.path);
        return false;
    }

    if (!GetIODesc())
        return false;

    NFE_LOG_SUCCESS("Shader '%s' compiled successfully", desc.path);
    return true;
}

bool Shader::Disassemble(bool html, Common::String& output)
{
    ID3DBlob* bytecode = mBytecode.Get();
    if (bytecode == nullptr)
    {
        NFE_LOG_ERROR("Shader is not compiled");
        return false;
    }

    UINT flags = 0;
    if (html)
        flags = D3D_DISASM_ENABLE_COLOR_CODE | D3D_DISASM_ENABLE_INSTRUCTION_NUMBERING;

    HRESULT hr;
    D3DPtr<ID3DBlob> disassembly;
    hr = D3D_CALL_CHECK(D3DDisassemble(bytecode->GetBufferPointer(), bytecode->GetBufferSize(),
                                       flags, 0, disassembly.GetPtr()));
    if (FAILED(hr))
        return false;

    const char* str = static_cast<const char*>(disassembly->GetBufferPointer());
    const uint32 len = static_cast<uint32>(Math::Min<size_t>(Common::String::MaxInternalLength, disassembly->GetBufferSize()));
    output = Common::String(str, len);

    return true;
}

ID3DBlob* Shader::GetBytecode() const
{
    return mBytecode.Get();
}

D3D12_SHADER_BYTECODE Shader::GetD3D12Bytecode() const
{
    D3D12_SHADER_BYTECODE result;

    if (mBytecode)
    {
        result.BytecodeLength = mBytecode->GetBufferSize();
        result.pShaderBytecode = mBytecode->GetBufferPointer();
    }
    else
    {
        result.BytecodeLength = 0;
        result.pShaderBytecode = nullptr;
    }

    return result;
}

bool Shader::GetIODesc()
{
    HRESULT hr;

    ID3DBlob* bytecode = mBytecode.Get();
    if (bytecode == nullptr)
    {
        NFE_LOG_ERROR("Shader is not compiled");
        return false;
    }

    // get reflector object
    D3DPtr<ID3D11ShaderReflection> reflection;
    hr = D3D_CALL_CHECK(D3DReflect(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), IID_PPV_ARGS(reflection.GetPtr())));
    if (FAILED(hr))
        return false;

    // TODO add support for ID3D12ShaderReflection for Shader Model 5.1

    // get shader descriptior via reflector
    D3D11_SHADER_DESC desc;
    hr = D3D_CALL_CHECK(reflection->GetDesc(&desc));
    if (FAILED(hr))
        return false;

    for (UINT i = 0; i < desc.BoundResources; ++i)
    {
        D3D11_SHADER_INPUT_BIND_DESC d3dBindingDesc;
        hr = D3D_CALL_CHECK(reflection->GetResourceBindingDesc(i, &d3dBindingDesc));
        if (FAILED(hr))
        {
            NFE_LOG_ERROR("Failed to parse resource binding, i = %d", i);
            return false;
        }

        ResBinding bindingDesc;
        bindingDesc.slot = d3dBindingDesc.BindPoint;

        switch (d3dBindingDesc.Type)
        {
        case D3D_SIT_CBUFFER:
            bindingDesc.type = ShaderResourceType::CBuffer;
            break;
        case D3D_SIT_TEXTURE:
            bindingDesc.type = ShaderResourceType::Texture;
            break;
        case D3D_SIT_SAMPLER:
            bindingDesc.type = ShaderResourceType::Sampler;
            break;
        case D3D_SIT_STRUCTURED:
            bindingDesc.type = ShaderResourceType::StructuredBuffer;
            break;
        case D3D_SIT_UAV_RWSTRUCTURED:
            bindingDesc.type = ShaderResourceType::WritableStructuredBuffer;
            break;
        case D3D_SIT_UAV_RWTYPED:
            bindingDesc.type = ShaderResourceType::WritableTexture;
            break;
        default:
            NFE_LOG_WARNING("Unsupported shader resource type (%d) at slot %d (name: '%s')",
                        d3dBindingDesc.Type, i, d3dBindingDesc.Name);
            continue;
        }

        Common::String name = d3dBindingDesc.Name;
        if (mResBindings.Exists(name))
        {
            NFE_LOG_ERROR("Multiple declarations of shader resource named '%s'", d3dBindingDesc.Name);
            return false;
        }

        mResBindings.Insert(std::move(name), bindingDesc);
    }

    return true;
}

int Shader::GetResourceSlotByName(const char* name)
{
    auto iter = mResBindings.Find(name);
    if (iter != mResBindings.End())
    {
        // TODO: verify resource type
        return iter->second.slot;
    }

    return -1;
}

} // namespace Renderer
} // namespace NFE
