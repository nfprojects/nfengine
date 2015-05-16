/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's shader
 */

#include "PCH.hpp"
#include "RendererD3D11.hpp"
#include "Logger.hpp"
#include "File.hpp"

namespace NFE {
namespace Renderer {

Shader::Shader()
    : mGeneric(nullptr)
    , mType(ShaderType::Unknown)
{
}

Shader::~Shader()
{
    switch (mType)
    {
        case ShaderType::Vertex:
            D3D_SAFE_RELEASE(mVS);
            break;
        case ShaderType::Geometry:
            D3D_SAFE_RELEASE(mGS);
            break;
        case ShaderType::Hull:
            D3D_SAFE_RELEASE(mHS);
            break;
        case ShaderType::Domain:
            D3D_SAFE_RELEASE(mDS);
            break;
        case ShaderType::Pixel:
            D3D_SAFE_RELEASE(mPS);
            break;
    }
}

bool Shader::Init(const ShaderDesc& desc)
{
    mType = desc.type;

    HRESULT hr;
    std::vector<char> str;
    size_t shaderSize = 0;
    const char* code = nullptr;

    if (desc.code == nullptr)
    {
        if (desc.path == nullptr)
        {
            LOG_ERROR("Shader code or path must be suplied");
            return false;
        }

        using namespace Common;
        File file(desc.path, AccessMode::Read);
        shaderSize = static_cast<size_t>(file.GetSize());
        str.resize(shaderSize + 1);

        if (file.Read(str.data(), shaderSize) != shaderSize)
            return false;
        str[shaderSize] = '\0';
        code = str.data();
    }
    else
    {
        code = desc.code;
        shaderSize = strlen(code);
    }

    DWORD flags = D3DCOMPILE_ENABLE_STRICTNESS;
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
            profileName = "vs_4_0";
            break;
        case ShaderType::Geometry:
            profileName = "gs_4_0";
            break;
        case ShaderType::Hull:
            profileName = "hs_4_0";
            break;
        case ShaderType::Domain:
            profileName = "ds_4_0";
            break;
        case ShaderType::Pixel:
            profileName = "ps_4_0";
            break;
        default:
            LOG_ERROR("Invalid shader type");
            return false;
    }

    ID3DBlob* errorsBuffer = nullptr;
    hr = D3DCompile(code, shaderSize, desc.path, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                    "main", profileName, flags, 0, &mBytecode, &errorsBuffer);

    if (errorsBuffer)
    {
        LOG_ERROR("Shader '%s' compilation output:\n%s", desc.path,
                  (char*)errorsBuffer->GetBufferPointer());
        errorsBuffer->Release();
    }

    if (FAILED(hr))
    {
        LOG_ERROR("Compilation of shader '%s' failed", desc.path);
        return false;
    }

    switch (mType)
    {
        case ShaderType::Vertex:
            hr = D3D_CALL_CHECK(gDevice->Get()->CreateVertexShader(mBytecode->GetBufferPointer(),
                                                                   mBytecode->GetBufferSize(),
                                                                   NULL, &mVS));
            break;
        case ShaderType::Geometry:
            hr = D3D_CALL_CHECK(gDevice->Get()->CreateGeometryShader(mBytecode->GetBufferPointer(),
                                                                     mBytecode->GetBufferSize(),
                                                                     NULL, &mGS));
            break;
        case ShaderType::Hull:
            hr = D3D_CALL_CHECK(gDevice->Get()->CreateHullShader(mBytecode->GetBufferPointer(),
                                                                 mBytecode->GetBufferSize(),
                                                                 NULL, &mHS));
            break;
        case ShaderType::Domain:
            hr = D3D_CALL_CHECK(gDevice->Get()->CreateDomainShader(mBytecode->GetBufferPointer(),
                                                                   mBytecode->GetBufferSize(),
                                                                   NULL, &mDS));
            break;
        case ShaderType::Pixel:
            hr = D3D_CALL_CHECK(gDevice->Get()->CreatePixelShader(mBytecode->GetBufferPointer(),
                                                                  mBytecode->GetBufferSize(),
                                                                  NULL, &mPS));
            break;
    }

    if (FAILED(hr))
    {
        mBytecode.reset();
        return false;
    }

    LOG_SUCCESS("Shader '%s' compiled successfully", desc.path)
    return true;
}

bool Shader::Disassemble(bool html, std::string& output)
{
    ID3DBlob* bytecode = mBytecode.get();
    if (bytecode == nullptr)
    {
        LOG_ERROR("Shader is not compiled");
        return false;
    }

    UINT flags = 0;
    if (html)
        flags = D3D_DISASM_ENABLE_COLOR_CODE | D3D_DISASM_ENABLE_INSTRUCTION_NUMBERING;

    HRESULT hr;
    D3DPtr<ID3DBlob> disassembly;
    hr = D3D_CALL_CHECK(D3DDisassemble(bytecode->GetBufferPointer(), bytecode->GetBufferSize(),
                                       flags, 0, &disassembly));
    if (FAILED(hr))
        return false;

    const char* str = static_cast<const char*>(disassembly->GetBufferPointer());
    output = std::string(str, disassembly->GetBufferSize());

    return true;
}

// TODO: what about separate functions for each shader type?
void* Shader::GetShaderObject() const
{
    return mGeneric;
}

ID3DBlob* Shader::GetBytecode() const
{
    return mBytecode.get();
}

bool Shader::GetIODesc(ShaderIODesc& result)
{
    HRESULT hr;
    result.resBinding.clear();

    ID3DBlob* bytecode = mBytecode.get();
    if (bytecode == nullptr)
    {
        LOG_ERROR("Shader is not compiled");
        return false;
    }

    // get reflector object
    D3DPtr<ID3D11ShaderReflection> reflection;
    hr = D3D_CALL_CHECK(D3DReflect(bytecode->GetBufferPointer(), bytecode->GetBufferSize(),
                                   IID_ID3D11ShaderReflection, (void**)&reflection));
    if (FAILED(hr))
        return false;

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
            LOG_ERROR("Failed to parse resource binding, i = %d", i);
            return false;
        }

        ShaderResBindingDesc bindingDesc;
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
        default:
            LOG_WARNING("Unsupported shader resource type (%d) at slot %d (name: '%s')",
                        d3dBindingDesc.Type, i, d3dBindingDesc.Name);
            continue;
        }

        std::string name = d3dBindingDesc.Name;
        result.resBinding[name] = bindingDesc;
    }

    return true;
}


} // namespace Renderer
} // namespace NFE
