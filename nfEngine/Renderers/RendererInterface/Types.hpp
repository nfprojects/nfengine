/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

namespace NFE {
namespace Renderer {

#define MAX_RENDER_TARGETS 8
#define MAX_MIPMAP_LEVELS 16

/**
 * GPU resources data format.
 */
enum class ElementFormat
{
    Unknown,

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

enum class TextureType
{
    Texture1D,
    Texture2D,
    Texture3D,
    TextureCube,
    Unknown
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

/**
 * GPU buffer access modes.
 */
enum class BufferAccess
{
    // TODO: this was taken form D3D11. Accesses may differ in OpenGL

    /**
     * Read-only resource, for example a static mesh or texture.
     * The resource content must be specified during creation.
     */
    GPU_ReadOnly,

    /**
     * Read-write resource, for example a texture used as a render target.
     * The content can't be accessed by the CPU.
     */
    GPU_ReadWrite,

    /**
     * Read-only resource, for example a constant buffer.
     * The content can be written by the CPU.
     */
    CPU_Write,

    /**
     * Readback resource, for example a screenshot texture.
     * The content can't be accessed directly by the CPU (only via Copy operations).
     * The data can be read by the CPU.
     */
    CPU_Read
};

/**
 * Alpha blending functions.
 */
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

/**
 * Alpha blending operators.
 */
enum class BlendOp
{
    Add,         //< A + B
    Subtract,    //< A - B
    RevSubtract, //< B - A
    Min,         //< min(A, B)
    Max          //< max(A, B)
};

/**
 * Polygon culling mode.
 */
enum class CullMode
{
    None, //< disable backface culling
    CW,   //< cull clockwise faces (default)
    CCW,  //< cull counter-clockwise faces
};

/**
 * Polygon filling mode.
 */
enum class FillMode
{
    Solid,    //< fill triangles (default)
    Wireframe //< draw wireframe only
};

/*
 * Comparison function.
 */
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

/**
 * Texture adressing mode for coordinates outside range [0.0, 1.0)
 */
enum class TextureWrapMode
{
    Repeat,
    Clamp,
    Mirror,
    Border
};

/**
 * Texture minification filter.
 */
enum class TextureMinFilter
{
    Nearest,
    Linear
};

/**
 * Texture magnification filter.
 */
enum class TextureMagFilter
{
    Nearest,
    Linear
};

} // namespace Renderer
} // namespace NFE
