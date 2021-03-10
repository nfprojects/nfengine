/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of nfEngine to Direct3D 12 translations functions.
 */

#pragma once

#include "../RendererCommon/Types.hpp"
#include "Format.hpp"
#include "Engine/Common/nfCommon.hpp"


namespace NFE {
namespace Renderer {

DXGI_FORMAT TranslateFormat(Format format);
D3D12_COMPARISON_FUNC TranslateComparisonFunc(CompareFunc func);
D3D12_STENCIL_OP TranslateStencilOp(StencilOp op);
D3D12_BLEND TranslateBlendFunc(BlendFunc func);
D3D12_BLEND_OP TranslateBlendOp(BlendOp op);
D3D12_CULL_MODE TranslateCullMode(CullMode cullMode);
D3D12_FILL_MODE TranslateFillMode(FillMode fillMode);
D3D12_PRIMITIVE_TOPOLOGY TranslatePrimitiveType(PrimitiveType type, uint32 controlPoints);
D3D12_PRIMITIVE_TOPOLOGY_TYPE TranslatePrimitiveTopologyType(PrimitiveType type);
D3D12_FILTER TranslateFilterType(TextureMinFilter minFilter, TextureMagFilter magFilter,
                                 bool compare, bool anisotropic);
D3D12_TEXTURE_ADDRESS_MODE TranslateTextureAddressMode(TextureWrapMode mode);
bool TranslateShaderVisibility(ShaderType shaderType, D3D12_SHADER_VISIBILITY& visibility);
bool TranslateDepthBufferTypes(Format inFormat, DXGI_FORMAT& resFormat, DXGI_FORMAT& srvFormat, DXGI_FORMAT& dsvFormat);

} // namespace Renderer
} // namespace NFE
