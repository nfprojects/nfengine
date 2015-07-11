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

namespace NFE {
namespace Renderer {

VertexLayout::VertexLayout()
{
}

bool VertexLayout::Init(const VertexLayoutDesc& desc)
{
    D3D11_INPUT_ELEMENT_DESC elementDescs[16];

    int offset = 0;
    for (int i = 0; i < desc.numElements; ++i)
    {
        /*
         TODO: almost everything here is temporary:
         * custom offsets not suported
         * per instance vertex data not supported
        */
        D3D11_INPUT_ELEMENT_DESC& el = elementDescs[i];
        el.SemanticName = (i == 0) ? "POSITION" : "TEXCOORD";
        el.SemanticIndex = (i == 0) ? 0 : (i - 1);
        el.Format = TranslateElementFormat(desc.elements[i].format, desc.elements[i].size);
        el.InputSlot = 0;
        el.AlignedByteOffset = offset;
        el.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        el.InstanceDataStepRate = 0;

        offset += GetElementFormatSize(desc.elements[i].format) * desc.elements[i].size;
    }

    Shader* vertexShader = dynamic_cast<Shader*>(desc.vertexShader);
    ID3DBlob* byteCode = vertexShader->GetBytecode();
    HRESULT hr = D3D_CALL_CHECK(gDevice->Get()->CreateInputLayout(elementDescs, desc.numElements,
                                                                  byteCode->GetBufferPointer(),
                                                                  byteCode->GetBufferSize(),
                                                                  &mIL));
    return SUCCEEDED(hr);
}

} // namespace Renderer
} // namespace NFE
