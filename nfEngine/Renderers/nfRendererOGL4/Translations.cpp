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
    switch (type)
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

GLenum TranslateElementFormat(ElementFormat format, bool& normalized)
{
    // set normalized bool
    switch (format)
    {
    case ElementFormat::Int_8_norm:
    case ElementFormat::Int_16_norm:
    case ElementFormat::Uint_8_norm:
    case ElementFormat::Uint_16_norm:
        normalized = true;
        break;
    default:
        normalized = false;
    }

    switch (format)
    {
    case ElementFormat::Int_8:
    case ElementFormat::Int_8_norm:
        return GL_BYTE;
    case ElementFormat::Uint_8:
    case ElementFormat::Uint_8_norm:
        return GL_UNSIGNED_BYTE;
    case ElementFormat::Int_16:
    case ElementFormat::Int_16_norm:
        return GL_SHORT;
    case ElementFormat::Uint_16:
    case ElementFormat::Uint_16_norm:
        return GL_UNSIGNED_SHORT;
    case ElementFormat::Int_32:
        return GL_INT;
    case ElementFormat::Uint_32:
        return GL_UNSIGNED_INT;
    case ElementFormat::Float_16:
        return GL_HALF_FLOAT;
    case ElementFormat::Float_32:
        return GL_FLOAT;
    default:
        LOG_ERROR("Unsupported, or incorrect Element Format provided.");
        return GL_NONE;
    }
}

GLenum TranslatePrimitiveType(PrimitiveType type)
{
    switch (type)
    {
    case PrimitiveType::Points:
        return GL_POINTS;
    case PrimitiveType::Lines:
        return GL_LINES;
    case PrimitiveType::LinesStrip:
        return GL_LINE_STRIP;
    case PrimitiveType::Triangles:
        return GL_TRIANGLES;
    case PrimitiveType::TrianglesStrip:
        return GL_TRIANGLE_STRIP;
    default:
        return GL_NONE;
    }
}

GLenum TranslateIndexBufferFormat(IndexBufferFormat format)
{
    switch (format)
    {
    case IndexBufferFormat::Uint16:
        return GL_UNSIGNED_SHORT;
    case IndexBufferFormat::Uint32:
        return GL_UNSIGNED_INT;
    default:
        LOG_ERROR("Incorrect Index Buffer Format provided.");
        return GL_NONE;
    }
}

GLsizei GetElementFormatSize(ElementFormat format)
{
    switch (format)
    {
    case ElementFormat::Int_8:
    case ElementFormat::Int_8_norm:
    case ElementFormat::Uint_8:
    case ElementFormat::Uint_8_norm:
        return 1;
    case ElementFormat::Int_16:
    case ElementFormat::Int_16_norm:
    case ElementFormat::Uint_16:
    case ElementFormat::Uint_16_norm:
    case ElementFormat::Float_16:
        return 2;
    case ElementFormat::Int_32:
    case ElementFormat::Uint_32:
    case ElementFormat::Float_32:
        return 4;
    default:
        LOG_ERROR("Unsupported, or incorrect Element Format provided.");
        return 0;
    }
}

} // namespace Renderer
} // namespace NFE
