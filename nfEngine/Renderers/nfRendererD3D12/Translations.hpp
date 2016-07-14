/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of nfEngine to Direct3D 12 translations functions.
 */

#pragma once

#include "../RendererInterface/Types.hpp"
#include "nfCommon/nfCommon.hpp"


namespace NFE {
namespace Renderer {

DXGI_FORMAT TranslateElementFormat(ElementFormat format, int size);
D3D12_COMPARISON_FUNC TranslateComparisonFunc(CompareFunc func);
D3D12_STENCIL_OP TranslateStencilOp(StencilOp op);
D3D12_BLEND TranslateBlendFunc(BlendFunc func);
D3D12_BLEND_OP TranslateBlendOp(BlendOp op);
D3D12_PRIMITIVE_TOPOLOGY TranslatePrimitiveType(PrimitiveType type, uint32 controlPoints);
D3D12_PRIMITIVE_TOPOLOGY_TYPE TranslatePrimitiveTopologyType(PrimitiveType type);

} // namespace Renderer
} // namespace NFE
