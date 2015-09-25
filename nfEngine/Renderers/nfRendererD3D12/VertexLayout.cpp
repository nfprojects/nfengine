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
    UNUSED(desc);
    return false;
}

} // namespace Renderer
} // namespace NFE
