/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of nfEngine to OpenGL 4 translations functions.
 */

#include "PCH.hpp"

#include "Translations.hpp"
#include "Extensions.hpp"

namespace NFE {
namespace Renderer {

GLuint TranslateShaderType(ShaderType type)
{
    switch(type)
    {
    case ShaderType::Vertex:
        return GL_VERTEX_SHADER;
    case ShaderType::Pixel:
        return GL_FRAGMENT_SHADER;
    case ShaderType::Geometry:
        return GL_GEOMETRY_SHADER;
    case ShaderType::Hull:
        return GL_TESS_CONTROL_SHADER;
    case ShaderType::Domain:
        return GL_TESS_EVALUATION_SHADER;
    default:
        return 0;
    }
}

} // namespace Renderer
} // namespace NFE
