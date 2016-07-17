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

DXGI_FORMAT TranslateElementFormat(ElementFormat format);
DXGI_FORMAT TranslateDepthFormat(DepthBufferFormat format);
D3D12_COMPARISON_FUNC TranslateComparisonFunc(CompareFunc func);
D3D12_STENCIL_OP TranslateStencilOp(StencilOp op);
D3D12_BLEND TranslateBlendFunc(BlendFunc func);
D3D12_BLEND_OP TranslateBlendOp(BlendOp op);
D3D12_PRIMITIVE_TOPOLOGY TranslatePrimitiveType(PrimitiveType type, uint32 controlPoints);
D3D12_PRIMITIVE_TOPOLOGY_TYPE TranslatePrimitiveTopologyType(PrimitiveType type);
D3D12_FILTER TranslateFilterType(TextureMinFilter minFilter, TextureMagFilter magFilter,
                                 bool compare, bool anisotropic);
D3D12_TEXTURE_ADDRESS_MODE TranslateTextureAddressMode(TextureWrapMode mode);
bool TranslateShaderVisibility(ShaderType shaderType, D3D12_SHADER_VISIBILITY& visibility);
bool TranslateDepthBufferTypes(DepthBufferFormat inFormat, DXGI_FORMAT& resFormat,
                               DXGI_FORMAT& srvFormat, DXGI_FORMAT& dsvFormat);

} // namespace Renderer
} // namespace NFE
