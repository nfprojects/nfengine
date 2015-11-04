/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of nfEngine to OpenGL 4 translations functions.
 */

#pragma once

#include "../RendererInterface/Types.hpp"

#include "Defines.hpp"

namespace NFE {
namespace Renderer {

GLuint TranslateShaderTypeToGLMacro(ShaderType type);
GLuint TranslateShaderTypeToGLBit(ShaderType type);
GLenum TranslateElementFormatToType(ElementFormat format, bool& normalized);
GLenum TranslateTexelSizeToFormat(int texelSize);
GLenum TranslateDepthFormatToType(DepthBufferFormat format);
GLenum TranslateDepthFormatToFormat(DepthBufferFormat format);
GLenum TranslateDepthFormatToInternalFormat(DepthBufferFormat format);
GLenum TranslatePrimitiveType(PrimitiveType type);
GLenum TranslateIndexBufferFormat(IndexBufferFormat format);
GLenum TranslateTextureMinFilter(TextureMinFilter minFilter);
GLenum TranslateTextureMagFilter(TextureMagFilter magFilter);
GLenum TranslateTextureWrapMode(TextureWrapMode wrapMode);
GLenum TranslateCompareFunc(CompareFunc func);
GLenum TranslateCullMode(CullMode mode);
GLenum TranslateFillMode(FillMode mode);
GLenum TranslateBlendFunc(BlendFunc func);
GLenum TranslateBlendOp(BlendOp op);
GLenum TranslateStencilOp(StencilOp op);
GLsizei GetElementFormatSize(ElementFormat format);

} // namespace Renderer
} // namespace NFE
