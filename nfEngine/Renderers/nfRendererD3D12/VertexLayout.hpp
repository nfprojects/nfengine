/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 render's vertex layout.
 */

#pragma once

#include "../RendererInterface/VertexLayout.hpp"
#include "Common.hpp"

namespace NFE {
namespace Renderer {

class VertexLayout : public IVertexLayout
{
    friend class PipelineState;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mElements;

public:
    VertexLayout();
    bool Init(const VertexLayoutDesc& desc);
};

} // namespace Renderer
} // namespace NFE
