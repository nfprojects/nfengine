/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of nfEngine to Direct3D 11 translations functions.
 */

#pragma once

#include "../RendererInterface/Types.hpp"

namespace NFE {
namespace Renderer {

int GetElementFormatSize(ElementFormat format);
DXGI_FORMAT TranslateElementFormat(ElementFormat format, int size);
D3D11_PRIMITIVE_TOPOLOGY TranslatePrimitiveType(PrimitiveType type);
D3D11_COMPARISON_FUNC TranslateComparisonFunc(CompareFunc func);
D3D11_TEXTURE_ADDRESS_MODE TranslateTextureWrapMode(TextureWrapMode mode);
bool TranslateDepthBufferTypes(DepthBufferFormat inFormat, DXGI_FORMAT& resFormat,
                               DXGI_FORMAT& srvFormat, DXGI_FORMAT& dsvFormat);
D3D11_FILTER TranslateFilterType(TextureMinFilter minFilter, TextureMagFilter magFilter,
                                 bool compare, bool anisotropic);
D3D11_BLEND TranslateBlendFunc(BlendFunc func);
D3D11_BLEND_OP TranslateBlendOp(BlendOp op);

} // namespace Renderer
} // namespace NFE
