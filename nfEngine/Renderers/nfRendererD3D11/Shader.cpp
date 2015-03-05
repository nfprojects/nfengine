/**
 * @file    Shader.cpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's shader
 */

#include "stdafx.hpp"
#include "RendererD3D11.hpp"

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
    size_t shaderSize = strlen(desc.code);
    
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
        return false;
    }

    ID3DBlob* errorsBuffer = nullptr;
    hr = D3DCompile(desc.code, shaderSize, desc.name, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                    "main", profileName, flags, 0, &mBytecode, &errorsBuffer);

    if (errorsBuffer)
    {
        Log((char*)errorsBuffer->GetBufferPointer());
        errorsBuffer->Release();
    }

    if (FAILED(hr))
        return false;

    // TODO: verify output of Create*Shader() calls
    switch (mType)
    {
    case ShaderType::Vertex:
        gDevice->Get()->CreateVertexShader(mBytecode->GetBufferPointer(),
                                           mBytecode->GetBufferSize(), NULL, &mVS);
        break;
    case ShaderType::Geometry:
        gDevice->Get()->CreateGeometryShader(mBytecode->GetBufferPointer(),
                                             mBytecode->GetBufferSize(), NULL, &mGS);
        break;
    case ShaderType::Hull:
        gDevice->Get()->CreateHullShader(mBytecode->GetBufferPointer(),
                                         mBytecode->GetBufferSize(), NULL, &mHS);
        break;
    case ShaderType::Domain:
        gDevice->Get()->CreateDomainShader(mBytecode->GetBufferPointer(),
                                           mBytecode->GetBufferSize(), NULL, &mDS);
        break;
    case ShaderType::Pixel:
        gDevice->Get()->CreatePixelShader(mBytecode->GetBufferPointer(),
                                          mBytecode->GetBufferSize(), NULL, &mPS);
        break;
    }

    std::string msg = "Shader '" + std::string(desc.name) + "' created";
    Log(msg.c_str());

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

} // namespace Renderer
} // namespace NFE
