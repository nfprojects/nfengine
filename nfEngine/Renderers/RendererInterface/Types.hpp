/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

namespace NFE {
namespace Renderer {

/// predeclarations
class ITexture;
class IBuffer;
class ISampler;
class IBackbuffer;
class IResourceBindingLayout;
class ICommandList;
class ICommandBuffer;
class IPipelineState;
class IRenderTarget;
class IShader;
class IShaderProgram;
class IVertexLayout;

#define MAX_RENDER_TARGETS 8
#define MAX_MIPMAP_LEVELS 16

/// Clear flags passed to ICommandBuffer::Clear()
#define NFE_CLEAR_FLAG_TARGET  (1<<0)
#define NFE_CLEAR_FLAG_DEPTH   (1<<1)
#define NFE_CLEAR_FLAG_STENCIL (1<<2)

/**
 * GPU resources data format.
 */
enum class ElementFormat
{
    Unknown,

    Float_32,     //< 32-bit floating-point values
    Int_32,       //< 32-bit signed integer values
    Uint_32,      //< 32-bit unsigned integer values

    Float_16,     //< 16-bit floating-point values
    Int_16,       //< 16-bit signed integer values
    Uint_16,      //< 16-bit unsigned integer values
    Int_16_norm,  //< 16-bit signed integer values scaled to [-1.0, 1.0] range
    Uint_16_norm, //< 16-bit unsigned integer values scaled to [0.0, 1.0] range

    Int_8,        //< 8-bit signed integer values
    Uint_8,       //< 8-bit unsigned integer values
    Int_8_norm,   //< 8-bit signed integer values scaled to [-1.0, 1.0] range
    Uint_8_norm,  //< 8-bit unsigned integer values scaled to [0.0, 1.0] range

    /// Block Compressed formats
    BC1,          //< aka. DXT1;  unsigned, normalized RGB values; 8 bytes per 4x4 block
    BC2,          //< aka. DXT3;  unsigned, normalized RGBA values; 16 bytes per 4x4 block
    BC3,          //< aka. DXT5;  unsigned, normalized RGBA values; 16 bytes per 4x4 block
    BC4,          //< aka. RGTC1; unsigned, normalized Red values; 8 bytes per 4x4 block
    BC4_signed,   //< aka. RGTC1; signed, normalized Red values; 8 bytes per 4x4 block
    BC5,          //< aka. RGTC2; unsigned, normalized Red and Green values; 16 bytes per 4x4 block
    BC5_signed,   //< aka. RGTC2; signed, normalized Red and Green values; 16 bytes per 4x4 block
    BC6H,         //< aka. BPTC;  unsigned, floating-point RGB values; 16 bytes per 4x4 block
    BC6H_signed,  //< aka. BPTC;  signed, floating-point RGB values; 16 bytes per 4x4 block
    BC7,          //< aka. BPTC;  unsigned, normalized RGB/RGBA values; 16 bytes per 4x4 block
};

/**
 * Depth buffer format
 */
enum class DepthBufferFormat
{
    Depth16,          // 16-bit depth buffer
    Depth24_Stencil8, // 24-bit depth buffer + 8-bit stencil buffer
    Depth32,          // 32-bit depth buffer
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
    Unknown = -1,
    Vertex  = 0,
    Hull,       //< aka. "tessellation control"
    Domain,     //< aka. "tessellation eveluation"
    Geometry,
    Pixel,      //< aka. "fragment"
    Number,     //< number of shader types
    All = Number
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
 * GPU resource mapping type.
 */
enum class MapType
{
    WriteOnly,
    ReadOnly,
    ReadWrite
};

/**
 * Data format in a index buffer.
 */
enum class IndexBufferFormat
{
    Uint16,
    Uint32
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
    Disabled, //< disable backface culling
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
    EqualGreater,
    Greater,
    NotEqual,
    Pass
};

enum class StencilOp
{
    Keep,          //< Keep the old value
    Zero,          //< Set value to zero
    Replace,       //< Set to reference value
    Increment,     //< Increment by one (with saturation).
    IncrementWrap, //< Increment by one (with wrapping).
    Decrement,     //< Decrement by one (with saturation).
    DecrementWrap, //< Decrement by one (with wrapping).
    Invert         //< Invert the value
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
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear
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
