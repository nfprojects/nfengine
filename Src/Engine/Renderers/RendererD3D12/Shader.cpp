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
#include "Engine/Common/Containers/DynArray.hpp"
#include "Engine/Common/System/Windows/Common.hpp"


namespace {

struct D3DWideMacro
{
    NFE::Common::Utf16String name;
    NFE::Common::Utf16String value;
};

} // namespace


namespace NFE {
namespace Renderer {

Shader::Shader()
    : mType(ShaderType::Unknown)
{}

bool Shader::Init(const ShaderDesc& desc)
{
    mType = desc.type;

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
        File file(desc.path, AccessMode::Read);
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

    Common::String macrosStr;
    Common::DynArray<D3DWideMacro> d3dWideMacros; // only used as wchar_t string container
    Common::DynArray<DxcDefine> d3dMacros;
    if (desc.macrosNum > 0)
    {
        d3dWideMacros.Resize(desc.macrosNum);
        d3dMacros.Resize(desc.macrosNum);
        for (uint32 i = 0; i < desc.macrosNum; ++i)
        {
            Common::UTF8ToUTF16(desc.macros[i].name, d3dWideMacros[i].name);
            Common::UTF8ToUTF16(desc.macros[i].value, d3dWideMacros[i].value);
            d3dMacros[i].Name = d3dWideMacros[i].name.c_str();
            d3dMacros[i].Value = d3dWideMacros[i].value.c_str();

            macrosStr += Common::String('\'') + desc.macros[i].name + "' = '" + desc.macros[i].value + '\'';
            if (i < desc.macrosNum - 1)
                macrosStr += ", ";
        }
    }

    NFE_LOG_INFO("Compiling shader '%s' with macros: [%s]...", desc.path, macrosStr.Str());

    if (!gDevice->GetShaderCompiler().Compile(code, static_cast<uint32>(shaderSize), desc.path, mType, d3dMacros, mBytecode))
    {
        NFE_LOG_ERROR("Failed to compile shader %s", desc.path);
        return false;
    }

    if (!GetIODesc())
        return false;

    NFE_LOG_SUCCESS("Shader '%s' compiled successfully", desc.path);
    return true;
}

bool Shader::Disassemble(bool html, Common::String& output)
{
    NFE_UNUSED(html);

    D3DPtr<IDxcBlobEncoding> disassembly;
    if (!gDevice->GetShaderCompiler().Disassemble(mBytecode, disassembly))
        return false;

    const char* str = static_cast<const char*>(disassembly->GetBufferPointer());
    const uint32 len = static_cast<uint32>(Math::Min<size_t>(Common::String::MaxInternalLength, disassembly->GetBufferSize()));
    output = Common::String(str, len);

    return true;
}

IDxcBlob* Shader::GetBytecode() const
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

    IDxcBlob* bytecode = mBytecode.Get();
    if (bytecode == nullptr)
    {
        NFE_LOG_ERROR("Shader is not compiled");
        return false;
    }

    // get reflector object
    D3DPtr<ID3D12ShaderReflection> reflection;
    if (!gDevice->GetShaderCompiler().Reflect(mBytecode, reflection))
        return false;

    // get shader descriptior via reflector
    D3D12_SHADER_DESC desc;
    hr = D3D_CALL_CHECK(reflection->GetDesc(&desc));
    if (FAILED(hr))
        return false;

    for (UINT i = 0; i < desc.BoundResources; ++i)
    {
        D3D12_SHADER_INPUT_BIND_DESC d3dBindingDesc;
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

        Common::String name{ d3dBindingDesc.Name };
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
    auto iter = mResBindings.Find(Common::String(name));
    if (iter != mResBindings.End())
    {
        // TODO: verify resource type
        return iter->second.slot;
    }

    return -1;
}

} // namespace Renderer
} // namespace NFE
