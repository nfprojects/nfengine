/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's vertex layout
 */

#include "PCH.hpp"
#include "RendererD3D11.hpp"
#include "VertexLayout.hpp"
#include "Translations.hpp"
#include "Shader.hpp"
#include "../nfCommon/Logger.hpp"

namespace NFE {
namespace Renderer {

VertexLayout::VertexLayout()
{
}

bool VertexLayout::Init(const VertexLayoutDesc& desc)
{
    HRESULT hr;
    std::string vertexShaderCode;
    D3D11_INPUT_ELEMENT_DESC elementDescs[16];

    vertexShaderCode = "struct VertexShaderInput { ";
    for (int i = 0; i < desc.numElements; ++i)
    {
        D3D11_INPUT_ELEMENT_DESC& el = elementDescs[i];
        el.SemanticName = (i == 0) ? "POSITION" : "TEXCOORD";
        el.SemanticIndex = (i == 0) ? 0 : (i - 1);
        el.Format = TranslateElementFormat(desc.elements[i].format, desc.elements[i].size);
        el.InputSlot = desc.elements[i].vertexBufferId;
        el.AlignedByteOffset = desc.elements[i].offset;
        el.InputSlotClass = desc.elements[i].perInstance ?
                            D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
        el.InstanceDataStepRate = desc.elements[i].perInstance ?
                                  desc.elements[i].instanceDataStep : 0;

        if (i == 0)
            vertexShaderCode += "float" + std::to_string(desc.elements[i].size) +
                                " pos : POSITION; ";
        else
            vertexShaderCode += "float" + std::to_string(desc.elements[i].size) +
                                " param" + std::to_string(i - 1) + " : TEXCOORD" +
                                std::to_string(i - 1) + "; ";
    }

    vertexShaderCode += "}; float4 main(VertexShaderInput input) : SV_POSITION "
                        "{ return input.pos.xxxx; }";

    /// compile dummy vertex shader
    D3DPtr<ID3DBlob> errorsBuffer;
    D3DPtr<ID3DBlob> bytecode;
    hr = D3DCompile(vertexShaderCode.data(), vertexShaderCode.size(), nullptr, nullptr,
                    nullptr, "main", "vs_4_0", 0, 0, &bytecode, &errorsBuffer);
    if (errorsBuffer)
    {
        LOG_INFO("Dummy vertex shader compilation output:\n%s",
                 static_cast<char*>(errorsBuffer->GetBufferPointer()));
    }

    if (FAILED(hr))
    {
        LOG_ERROR("Dummy vertex shader compilation failed");
        return false;
    }

    hr = D3D_CALL_CHECK(gDevice->Get()->CreateInputLayout(elementDescs, desc.numElements,
                                                          bytecode->GetBufferPointer(),
                                                          bytecode->GetBufferSize(), &mIL));
    if (FAILED(hr))
        return false;

#ifdef D3D_DEBUGGING
    /// set debug name
    std::string bufferName = "NFE::Renderer::VertexLayout \"";
    if (desc.debugName)
        bufferName += desc.debugName;
    bufferName += '"';
    mIL->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(bufferName.length()),
                        bufferName.c_str());
#endif // D3D_DEBUGGING

    return true;
}

} // namespace Renderer
} // namespace NFE
