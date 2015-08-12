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

GLuint TranslateShaderType(ShaderType type);
GLenum TranslateElementFormat(ElementFormat format, bool& normalized);
GLenum TranslatePrimitiveType(PrimitiveType type);
GLenum TranslateIndexBufferFormat(IndexBufferFormat format);
GLsizei GetElementFormatSize(ElementFormat format);

} // namespace Renderer
} // namespace NFE
