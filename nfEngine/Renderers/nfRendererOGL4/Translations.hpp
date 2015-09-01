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

} // namespace Renderer
} // namespace NFE
