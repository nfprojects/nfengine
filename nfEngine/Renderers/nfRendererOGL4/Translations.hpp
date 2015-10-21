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
GLenum TranslateElementFormat(ElementFormat format, bool& normalized);
GLenum TranslatePrimitiveType(PrimitiveType type);
GLenum TranslateIndexBufferFormat(IndexBufferFormat format);
GLenum TranslateTextureMinFilter(TextureMinFilter minFilter);
GLenum TranslateTextureMagFilter(TextureMagFilter magFilter);
GLenum TranslateTextureWrapMode(TextureWrapMode wrapMode);
GLenum TranslateCompareFunc(CompareFunc func);
GLsizei GetElementFormatSize(ElementFormat format);

} // namespace Renderer
} // namespace NFE
