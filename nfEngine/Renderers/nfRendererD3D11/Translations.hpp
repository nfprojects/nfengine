/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of nfEngine to Direct3D 11 translations functions.
 */

#pragma once

#include "../RendererInterface/Types.hpp"
#include "../D3DCommon/Format.hpp"
#include "nfCommon/nfCommon.hpp"


namespace NFE {
namespace Renderer {

DXGI_FORMAT TranslateElementFormat(ElementFormat format);
D3D11_PRIMITIVE_TOPOLOGY TranslatePrimitiveType(PrimitiveType type, uint32 controlPoints = 0);
D3D11_COMPARISON_FUNC TranslateComparisonFunc(CompareFunc func);
D3D11_TEXTURE_ADDRESS_MODE TranslateTextureWrapMode(TextureWrapMode mode);
bool TranslateDepthBufferTypes(DepthBufferFormat inFormat, DXGI_FORMAT& resFormat,
                               DXGI_FORMAT& srvFormat, DXGI_FORMAT& dsvFormat);
D3D11_FILTER TranslateFilterType(TextureMinFilter minFilter, TextureMagFilter magFilter,
                                 bool compare, bool anisotropic);
D3D11_BLEND TranslateBlendFunc(BlendFunc func);
D3D11_BLEND_OP TranslateBlendOp(BlendOp op);
D3D11_STENCIL_OP TranslateStencilOp(StencilOp op);

} // namespace Renderer
} // namespace NFE
