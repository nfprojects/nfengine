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

GLuint TranslateShaderTypeToGLMacro(ShaderType type)
{
    switch (type)
    {
    case ShaderType::Vertex:   return GL_VERTEX_SHADER;
    case ShaderType::Pixel:    return GL_FRAGMENT_SHADER;
    case ShaderType::Geometry: return GL_GEOMETRY_SHADER;
    case ShaderType::Hull:     return GL_TESS_CONTROL_SHADER;
    case ShaderType::Domain:   return GL_TESS_EVALUATION_SHADER;
    default:                   return 0;
    }
}

GLuint TranslateShaderTypeToGLBit(ShaderType type)
{
    switch(type)
    {
    case ShaderType::Vertex:   return GL_VERTEX_SHADER_BIT;
    case ShaderType::Pixel:    return GL_FRAGMENT_SHADER_BIT;
    case ShaderType::Geometry: return GL_GEOMETRY_SHADER_BIT;
    case ShaderType::Hull:     return GL_TESS_CONTROL_SHADER_BIT;
    case ShaderType::Domain:   return GL_TESS_EVALUATION_SHADER_BIT;
    default:                   return GL_NONE;
    }
}

GLenum TranslateElementFormatToType(ElementFormat format, bool& normalized)
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
    case ElementFormat::Int_8:          return GL_BYTE;
    case ElementFormat::Int_8_norm:     return GL_BYTE;
    case ElementFormat::Uint_8:         return GL_UNSIGNED_BYTE;
    case ElementFormat::Uint_8_norm:    return GL_UNSIGNED_BYTE;
    case ElementFormat::Int_16:         return GL_SHORT;
    case ElementFormat::Int_16_norm:    return GL_SHORT;
    case ElementFormat::Uint_16:        return GL_UNSIGNED_SHORT;
    case ElementFormat::Uint_16_norm:   return GL_UNSIGNED_SHORT;
    case ElementFormat::Int_32:         return GL_INT;
    case ElementFormat::Uint_32:        return GL_UNSIGNED_INT;
    case ElementFormat::Float_16:       return GL_HALF_FLOAT;
    case ElementFormat::Float_32:       return GL_FLOAT;
    default:                            return GL_NONE;
    }
}

GLenum TranslateTexelSizeToFormat(int texelSize)
{
    switch (texelSize)
    {
    case 1:     return GL_RED;
    case 2:     return GL_RG;
    case 3:     return GL_RGB;
    case 4:     return GL_RGBA;
    default:    return GL_NONE;
    }
}

GLenum TranslateDepthFormatToType(DepthBufferFormat format)
{
    switch (format)
    {
    case DepthBufferFormat::Depth16:            return GL_UNSIGNED_SHORT;
    case DepthBufferFormat::Depth24_Stencil8:   return GL_UNSIGNED_INT_24_8;
    case DepthBufferFormat::Depth32:            return GL_FLOAT;
    default:                                    return GL_NONE;
    }
}

GLenum TranslateDepthFormatToFormat(DepthBufferFormat format)
{
    switch (format)
    {
    case DepthBufferFormat::Depth16:            return GL_DEPTH_COMPONENT;
    case DepthBufferFormat::Depth24_Stencil8:   return GL_DEPTH_STENCIL;
    case DepthBufferFormat::Depth32:            return GL_DEPTH_COMPONENT;
    default:                                    return GL_NONE;
    }
}

GLenum TranslateDepthFormatToInternalFormat(DepthBufferFormat format)
{
    switch (format)
    {
    case DepthBufferFormat::Depth16:            return GL_DEPTH_COMPONENT16;
    case DepthBufferFormat::Depth24_Stencil8:   return GL_DEPTH24_STENCIL8;
    case DepthBufferFormat::Depth32:            return GL_DEPTH_COMPONENT32F;
    default:                                    return GL_NONE;
    }
}

GLenum TranslatePrimitiveType(PrimitiveType type)
{
    switch (type)
    {
    case PrimitiveType::Points:         return GL_POINTS;
    case PrimitiveType::Lines:          return GL_LINES;
    case PrimitiveType::LinesStrip:     return GL_LINE_STRIP;
    case PrimitiveType::Triangles:      return GL_TRIANGLES;
    case PrimitiveType::TrianglesStrip: return GL_TRIANGLE_STRIP;
    default:                            return GL_NONE;
    }
}

GLenum TranslateIndexBufferFormat(IndexBufferFormat format)
{
    switch (format)
    {
    case IndexBufferFormat::Uint16: return GL_UNSIGNED_SHORT;
    case IndexBufferFormat::Uint32: return GL_UNSIGNED_INT;
    default:                        return GL_NONE;
    }
}

GLenum TranslateTextureMinFilter(TextureMinFilter minFilter)
{
    switch (minFilter)
    {
    case TextureMinFilter::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
    case TextureMinFilter::LinearMipmapNearest:  return GL_LINEAR_MIPMAP_NEAREST;
    case TextureMinFilter::NearestMipmapLinear:  return GL_NEAREST_MIPMAP_LINEAR;
    case TextureMinFilter::LinearMipmapLinear:   return GL_LINEAR_MIPMAP_LINEAR;
    default:                                     return GL_NONE;
    }
}

GLenum TranslateTextureMagFilter(TextureMagFilter magFilter)
{
    switch (magFilter)
    {
    case TextureMagFilter::Nearest: return GL_NEAREST;
    case TextureMagFilter::Linear:  return GL_LINEAR;
    default:                        return GL_NONE;
    }
}

GLenum TranslateTextureWrapMode(TextureWrapMode wrapMode)
{
    switch (wrapMode)
    {
    case TextureWrapMode::Repeat: return GL_REPEAT;
    case TextureWrapMode::Clamp:  return GL_CLAMP_TO_EDGE;
    case TextureWrapMode::Mirror: return GL_MIRRORED_REPEAT;
    case TextureWrapMode::Border: return GL_CLAMP_TO_BORDER;
    default:                      return GL_NONE;
    }
}

GLenum TranslateCompareFunc(CompareFunc func)
{
    switch (func)
    {
    case CompareFunc::Never:        return GL_NEVER;
    case CompareFunc::Less:         return GL_LESS;
    case CompareFunc::LessEqual:    return GL_LEQUAL;
    case CompareFunc::Equal:        return GL_EQUAL;
    case CompareFunc::EqualGreater: return GL_GEQUAL;
    case CompareFunc::Greater:      return GL_GREATER;
    case CompareFunc::NotEqual:     return GL_NOTEQUAL;
    case CompareFunc::Pass:         return GL_ALWAYS;
    default:                        return GL_NONE;
    }
}

GLenum TranslateCullMode(CullMode mode)
{
    // assumes CW culling mode is front (will be force-set by CommandBuffer)
    switch (mode)
    {
    case CullMode::Disabled: return GL_FRONT_AND_BACK;
    case CullMode::CW:       return GL_FRONT;
    case CullMode::CCW:      return GL_BACK;
    default:                 return GL_NONE;
    }
}

GLenum TranslateFillMode(FillMode mode)
{
    switch (mode)
    {
    case FillMode::Solid:     return GL_FILL;
    case FillMode::Wireframe: return GL_LINE;
    default:                  return GL_NONE;
    }
}

GLenum TranslateBlendFunc(BlendFunc func)
{
    switch (func)
    {
    case BlendFunc::Zero:              return GL_ZERO;
    case BlendFunc::One:               return GL_ONE;
    case BlendFunc::SrcColor:          return GL_SRC_COLOR;
    case BlendFunc::DestColor:         return GL_DST_COLOR;
    case BlendFunc::SrcAlpha:          return GL_SRC_ALPHA;
    case BlendFunc::DestAlpha:         return GL_DST_ALPHA;
    case BlendFunc::OneMinusSrcColor:  return GL_ONE_MINUS_SRC_COLOR;
    case BlendFunc::OneMinusDestColor: return GL_ONE_MINUS_DST_COLOR;
    case BlendFunc::OneMinusSrcAlpha:  return GL_ONE_MINUS_SRC_ALPHA;
    case BlendFunc::OneMinusDestAlpha: return GL_ONE_MINUS_DST_ALPHA;
    default:                           return GL_NONE;
    }
}

GLenum TranslateBlendOp(BlendOp op)
{
    switch (op)
    {
    case BlendOp::Add:         return GL_FUNC_ADD;
    case BlendOp::Subtract:    return GL_FUNC_SUBTRACT;
    case BlendOp::RevSubtract: return GL_FUNC_REVERSE_SUBTRACT;
    case BlendOp::Min:         return GL_MIN;
    case BlendOp::Max:         return GL_MAX;
    default:                   return GL_NONE;
    }
}

GLenum TranslateStencilOp(StencilOp op)
{
    switch(op)
    {
    case StencilOp::Keep:          return GL_KEEP;
    case StencilOp::Zero:          return GL_ZERO;
    case StencilOp::Replace:       return GL_REPLACE;
    case StencilOp::Increment:     return GL_INCR;
    case StencilOp::IncrementWrap: return GL_INCR_WRAP;
    case StencilOp::Decrement:     return GL_DECR;
    case StencilOp::DecrementWrap: return GL_DECR_WRAP;
    case StencilOp::Invert:        return GL_INVERT;
    default:                       return GL_NONE;
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
        return 0;
    }
}

} // namespace Renderer
} // namespace NFE
