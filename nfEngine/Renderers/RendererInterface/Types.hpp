/**
 * @file    Types.hpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

namespace NFE {
namespace Renderer {

#define MAX_RENDER_TARGETS 8

enum class ElementFormat
{
    Float_32,
    Int_32,
    Uint_32,

    Float_16,
    Int_16,
    Uint_16,
    Int_16_norm,
    Uint_16_norm,

    Int_8,
    Uint_8,
    Int_8_norm,
    Uint_8_norm,

    // TODO: compressed (BCx), etc.
};

enum class BufferType
{
    Vertex,
    Index,
    Constant
};

enum class ShaderType
{
    Unknown,
    Vertex,
    Hull,       //< aka. "tessellation control"
    Domain,     //< aka. "tessellation eveluation"
    Geometry,
    Pixel,      //< aka. "fragment"
};

enum class BufferAccess
{
    // TODO: this was taken form D3D11. Accesses may differ in OpenGL
    GPU_ReadOnly,
    GPU_ReadWrite,
    CPU_Write,
    CPU_Read
};

enum class BlendFunc
{
    Zero,
    One,
    SrcColor,
    DestColor,
    SrcAlpha,
    DestAlpha,
    OneMinusSrcColor,
    OneMinusDestColor,
    OneMinusSrcAlpha,
    OneMinusDestAlpha
    // TODO: support more, if needed
};

enum class BlendOp
{
    Add,         //< A + B
    Subtract,    //< A - B
    RevSubtract, //< B - A
    Min,         //< min(A, B)
    Max          //< max(A, B)
};

enum class CullMode
{
    None, //< disable backface culling
    CW,   //< cull clockwise faces (default)
    CCW,  //< cull counter-clockwise faces
};

enum class FillMode
{
    Solid,    //< fill triangles (default)
    Wireframe //< draw wireframe only
};

enum class CompareFunc
{
    Never,
    Less,
    LessEqual,
    Equal,
    EqualGrater,
    Grater,
    NotEqual,
    Always
};

enum class PrimitiveType
{
    Unknown,
    Points,
    Lines,
    LinesStrip,
    Triangles,
    TrianglesStrip
};

} // namespace Renderer
} // namespace NFE
