/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's vertex layout
 */

#include "PCH.hpp"
#include "RendererD3D12.hpp"
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
    for (int i = 0; i < desc.numElements; ++i)
    {
        D3D12_INPUT_ELEMENT_DESC el;
        el.SemanticName = (i == 0) ? "POSITION" : "TEXCOORD";
        el.SemanticIndex = (i == 0) ? 0 : (i - 1);
        el.Format = TranslateElementFormat(desc.elements[i].format, desc.elements[i].size);
        el.InputSlot = desc.elements[i].vertexBufferId;
        el.AlignedByteOffset = desc.elements[i].offset;
        el.InputSlotClass = desc.elements[i].perInstance ?
            D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA :
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        el.InstanceDataStepRate = desc.elements[i].perInstance ?
            desc.elements[i].instanceDataStep : 0;

        mElements.push_back(el);
    }

    return true;
}

} // namespace Renderer
} // namespace NFE
