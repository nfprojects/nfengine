#include "PCH.hpp"
#include "ShaderCompiler.hpp"

#include <Engine/Common/Logger/Logger.hpp>
#include <Engine/Common/Utils/StringUtils.hpp>
#include <Engine/Common/System/Windows/Common.hpp>

// TODO this should come from DXIL headers but they don't exist in Windows SDK
#define DXIL_FOURCC(ch0, ch1, ch2, ch3) (                            \
  (uint32_t)(uint8_t)(ch0)        | (uint32_t)(uint8_t)(ch1) << 8  | \
  (uint32_t)(uint8_t)(ch2) << 16  | (uint32_t)(uint8_t)(ch3) << 24   \
  )


namespace NFE {
namespace Renderer {

ShaderCompiler::ShaderCompiler()
    : mLibrary()
    , mCompiler()
    , mIncluder()
{
}

bool ShaderCompiler::Init()
{
    HRESULT hr = S_OK;

    hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(mLibrary.GetPtr()));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create DXC Library Instance: %x", hr);
        return false;
    }

    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(mCompiler.GetPtr()));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create DXC Compiler Instance: %x", hr);
        return false;
    }

    hr = mLibrary->CreateIncludeHandler(mIncluder.GetPtr());
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create DXC Include Handler: %x", hr);
        return false;
    }

    return true;
}

bool ShaderCompiler::Compile(const char* source, uint32 sourceSize, const char* sourceName, ShaderType type,
                             const Common::DynArray<DxcDefine>& defines, D3DPtr<IDxcBlob>& output)
{
    HRESULT hr = S_OK;

    // TODO: max profile level detection
    const wchar_t* profileName;
    switch (type)
    {
    case ShaderType::Vertex:
        profileName = L"vs_6_0";
        break;
    case ShaderType::Geometry:
        profileName = L"gs_6_0";
        break;
    case ShaderType::Hull:
        profileName = L"hs_6_0";
        break;
    case ShaderType::Domain:
        profileName = L"ds_6_0";
        break;
    case ShaderType::Pixel:
        profileName = L"ps_6_0";
        break;
    case ShaderType::Compute:
        profileName = L"cs_6_0";
        break;
    default:
        NFE_LOG_ERROR("Invalid shader type");
        return false;
    }

    IDxcBlobEncoding* srcBlobPtr;
    hr = mLibrary->CreateBlobWithEncodingOnHeapCopy(source, sourceSize, CP_UTF8, &srcBlobPtr);
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create shader source DXC blob: %x\n", hr);
        return false;
    }

    D3DPtr<IDxcBlobEncoding> srcBlob(srcBlobPtr);

    Common::DynArray<const wchar_t*> arguments;
    arguments.EmplaceBack(L"-Ges");
#ifdef _DEBUG
    arguments.EmplaceBack(L"-Od");
    arguments.EmplaceBack(L"-Zi");
#else
    arguments.EmplaceBack(L"-O3");
#endif

    Common::Utf16String wideSourceName;
    Common::UTF8ToUTF16(sourceName, wideSourceName);

    D3DPtr<IDxcOperationResult> result;
    IDxcOperationResult* resultPtr;
    hr = mCompiler->Compile(srcBlob.Get(), wideSourceName.c_str(),
                            L"main", profileName,
                            arguments.Data(), arguments.Size(),
                            defines.Data(), defines.Size(),
                            mIncluder.Get(),
                            &resultPtr);
    if (SUCCEEDED(hr))
    {
        result.Reset(resultPtr);
        result->GetStatus(&hr);
    }

    if (FAILED(hr))
    {
        if (result)
        {
            IDxcBlobEncoding* errorBlob;
            hr = result->GetErrorBuffer(&errorBlob);
            if (SUCCEEDED(hr) && errorBlob)
            {
                NFE_LOG_ERROR("Failed to compile shader %s: %s", sourceName, errorBlob->GetBufferPointer());
                return false;
            }
            else
            {
                NFE_LOG_ERROR("Failed to get error blob: %x\n", hr);
                return false;
            }
        }
        else
        {
            NFE_LOG_ERROR("DXC Compile call failed: %x\n", hr);
        }
    }

    IDxcBlob* code;
    result->GetResult(&code);
    output.Reset(code);
    return true;
}

bool ShaderCompiler::Disassemble(const D3DPtr<IDxcBlob>& bytecode, D3DPtr<IDxcBlobEncoding>& disassembly)
{
    HRESULT hr = S_OK;

    IDxcBlob* bytecodePtr = bytecode.Get();
    if (bytecodePtr == nullptr)
    {
        NFE_LOG_ERROR("Shader is not compiled");
        return false;
    }

    IDxcBlobEncoding* disassemblyPtr;
    hr = mCompiler->Disassemble(bytecodePtr, &disassemblyPtr);
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to disassemble shader");
        return false;
    }

    disassembly.Reset(disassemblyPtr);
    return true;
}

bool ShaderCompiler::Reflect(const D3DPtr<IDxcBlob>& bytecode, D3DPtr<ID3D12ShaderReflection>& reflection)
{
    HRESULT hr = S_OK;

    IDxcBlob* bytecodePtr = bytecode.Get();
    if (bytecodePtr == nullptr)
    {
        NFE_LOG_ERROR("Shader is not compiled");
        return false;
    }

    D3DPtr<IDxcContainerReflection> containerReflection;
    hr = DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(containerReflection.GetPtr()));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create DXC Container Reflection: %x", hr);
        return false;
    }

    hr = containerReflection->Load(bytecodePtr);
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to load shader to DXC Container Reflection: %x", hr);
        return false;
    }

    // TODO this should come from DXIL headers but they don't exist in Windows SDK
    uint32 DXILKind = DXIL_FOURCC('D', 'X', 'I', 'L');

    uint32 shaderIdx = 0;
    hr = containerReflection->FindFirstPartKind(DXILKind, &shaderIdx);
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to find DXIL code in blob: %x", hr);
        return false;
    }

    hr = containerReflection->GetPartReflection(shaderIdx, IID_PPV_ARGS(reflection.GetPtr()));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to acquire shader reflection: %x", hr);
        return false;
    }

    return true;
}

} // namespace Renderer
} // namespace NFE
