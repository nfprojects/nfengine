/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "nfCommon/nfCommon.hpp"


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
class IVertexLayout;

#define MAX_RENDER_TARGETS 8
#define MAX_MIPMAP_LEVELS 16


/**
 * GPU resources data format.
 */
enum class ElementFormat
{
    Unknown,

    /// single 8-bit values
    R8_U_Int,
    R8_S_Int,
    R8_U_Norm,
    R8_S_Norm,

    /// single 16-bit values
    R16_U_Int,
    R16_S_Int,
    R16_U_Norm,
    R16_S_Norm,
    R16_Float,

    /// single 32-bit values
    R32_U_Int,
    R32_S_Int,
    R32_Float,

    /// double 8-bit values
    R8G8_U_Int,
    R8G8_S_Int,
    R8G8_U_Norm,
    R8G8_S_Norm,

    /// double 16-bit values
    R16G16_U_Int,
    R16G16_S_Int,
    R16G16_U_Norm,
    R16G16_S_Norm,
    R16G16_Float,

    /// double 32-bit values
    R32G32_U_Int,
    R32G32_S_Int,
    R32G32_Float,

    /// tripple 32-bit values
    R32G32B32_U_Int,
    R32G32B32_S_Int,
    R32G32B32_Float,

    /// quadruple 8-bit values
    R8G8B8A8_U_Int,
    R8G8B8A8_S_Int,
    R8G8B8A8_U_Norm,
    R8G8B8A8_U_Norm_sRGB,
    R8G8B8A8_S_Norm,
    B8G8R8A8_U_Norm,
    B8G8R8A8_U_Norm_sRGB,

    /// quadruple 16-bit values
    R16G16B16A16_U_Int,
    R16G16B16A16_S_Int,
    R16G16B16A16_U_Norm,
    R16G16B16A16_S_Norm,
    R16G16B16A16_Float,

    /// quadruple 32-bit values
    R32G32B32A32_U_Int,
    R32G32B32A32_S_Int,
    R32G32B32A32_Float,

    /// packed types
    B5G6R5_U_norm,
    B5G5R5A1_U_Norm,
    B4G4R4A4_U_Norm,
    R11G11B10_Float,
    R10G10B10A2_U_Norm,
    R10G10B10A2_U_Int,
    R9G9B9E5_Float,     // shared exponent RGB values

    /// Block Compressed formats
    BC1_U_Norm,         //< aka. DXT1; 8 bytes per 4x4 block
    BC1_U_Norm_sRGB,    //< aka. DXT1; 8 bytes per 4x4 block
    BC2_U_Norm,         //< aka. DXT3; 16 bytes per 4x4 block
    BC2_U_Norm_sRGB,    //< aka. DXT3; 16 bytes per 4x4 block
    BC3_U_Norm,         //< aka. DXT5; 16 bytes per 4x4 block
    BC3_U_Norm_sRGB,    //< aka. DXT5; 16 bytes per 4x4 block
    BC4_U_Norm,         //< aka. RGTC1; 8 bytes per 4x4 block
    BC4_S_Norm,         //< aka. RGTC1; 8 bytes per 4x4 block
    BC5_U_Norm,         //< aka. RGTC2; unsigned normalized Red and Green values; 16 bytes per 4x4 block
    BC5_S_Norm,         //< aka. RGTC2; signed normalized Red and Green values; 16 bytes per 4x4 block
    BC6H_U_Float,       //< aka. BPTC;  unsigned, floating-point RGB values; 16 bytes per 4x4 block
    BC6H_S_Float,       //< aka. BPTC;  signed, floating-point RGB values; 16 bytes per 4x4 block
    BC7_U_Norm,         //< aka. BPTC;  unsigned, normalized RGB/RGBA values; 16 bytes per 4x4 block
    BC7_U_Norm_sRGB,    //< aka. BPTC;  unsigned, normalized RGB/RGBA values; 16 bytes per 4x4 block
};

/**
 * Depth buffer format
 */
enum class DepthBufferFormat
{
    Unknown,
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
 * GPU buffer modes.
 */
enum class BufferMode
{
    /**
     * Read-only resource, for example a static mesh or texture.
     * The resource content must be specified during creation.
     */
    Static,

    /**
     * Read-only resource, for example a constant buffer.
     * The content can be written by the CPU.
     */
    Dynamic,

    /**
     * Frequently written-to read-only resource.
     * The content can be written by the CPU. Assumes the data will be refreshed at least every frame.
     * This mode uses no actual Resource/Buffer allocation. Instead, internal Ring Buffer is used
     * to write data.
     */
    Volatile,

    /**
     * Read-write resource, for example a texture used as a render target.
     * The content can't be accessed by the CPU.
     */
    GPUOnly,

    /**
     * Readback resource, for example a screenshot texture.
     * The content can't be accessed directly by the CPU (only via Copy operations).
     * The data can be read by the CPU.
     */
    Readback
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
    TrianglesStrip,
    Patch,          //< enables tessellation
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


/**
 * Get format pixel channels.
 */
NFE_INLINE uint32 GetElementFormatChannels(ElementFormat format)
{
    switch (format)
    {
    case ElementFormat::R8_U_Int:
    case ElementFormat::R8_S_Int:
    case ElementFormat::R8_U_Norm:
    case ElementFormat::R8_S_Norm:
    case ElementFormat::R16_U_Int:
    case ElementFormat::R16_S_Int:
    case ElementFormat::R16_U_Norm:
    case ElementFormat::R16_S_Norm:
    case ElementFormat::R16_Float:
    case ElementFormat::R32_U_Int:
    case ElementFormat::R32_S_Int:
    case ElementFormat::R32_Float:
        return 1;

    case ElementFormat::R8G8_U_Int:
    case ElementFormat::R8G8_S_Int:
    case ElementFormat::R8G8_U_Norm:
    case ElementFormat::R8G8_S_Norm:
    case ElementFormat::R16G16_U_Int:
    case ElementFormat::R16G16_S_Int:
    case ElementFormat::R16G16_U_Norm:
    case ElementFormat::R16G16_S_Norm:
    case ElementFormat::R16G16_Float:
    case ElementFormat::R32G32_U_Int:
    case ElementFormat::R32G32_S_Int:
    case ElementFormat::R32G32_Float:
        return 2;

    case ElementFormat::R32G32B32_U_Int:
    case ElementFormat::R32G32B32_S_Int:
    case ElementFormat::R32G32B32_Float:
    case ElementFormat::B5G6R5_U_norm:
    case ElementFormat::R11G11B10_Float:
    case ElementFormat::R9G9B9E5_Float:
        return 3;

    case ElementFormat::R8G8B8A8_U_Int:
    case ElementFormat::R8G8B8A8_S_Int:
    case ElementFormat::R8G8B8A8_U_Norm:
    case ElementFormat::R8G8B8A8_U_Norm_sRGB:
    case ElementFormat::R8G8B8A8_S_Norm:
    case ElementFormat::B8G8R8A8_U_Norm:
    case ElementFormat::B8G8R8A8_U_Norm_sRGB:
    case ElementFormat::R16G16B16A16_U_Int:
    case ElementFormat::R16G16B16A16_S_Int:
    case ElementFormat::R16G16B16A16_U_Norm:
    case ElementFormat::R16G16B16A16_S_Norm:
    case ElementFormat::R16G16B16A16_Float:
    case ElementFormat::R32G32B32A32_U_Int:
    case ElementFormat::R32G32B32A32_S_Int:
    case ElementFormat::R32G32B32A32_Float:
    case ElementFormat::B5G5R5A1_U_Norm:
    case ElementFormat::B4G4R4A4_U_Norm:
    case ElementFormat::R10G10B10A2_U_Norm:
    case ElementFormat::R10G10B10A2_U_Int:
        return 4;

    case ElementFormat::BC1_U_Norm:
    case ElementFormat::BC1_U_Norm_sRGB:
    case ElementFormat::BC2_U_Norm:
    case ElementFormat::BC2_U_Norm_sRGB:
    case ElementFormat::BC3_U_Norm:
    case ElementFormat::BC3_U_Norm_sRGB:
        return 4;

    case ElementFormat::BC4_U_Norm:
    case ElementFormat::BC4_S_Norm:
        return 1;

    case ElementFormat::BC5_U_Norm:
    case ElementFormat::BC5_S_Norm:
        return 2;

    case ElementFormat::BC6H_U_Float:
    case ElementFormat::BC6H_S_Float:
    case ElementFormat::BC7_U_Norm:
    case ElementFormat::BC7_U_Norm_sRGB:
        return 3;
    }

    return 0;
}

/**
 * Get bytes per element.
 */
NFE_INLINE uint32 GetElementFormatSize(ElementFormat format)
{
    switch (format)
    {
    case ElementFormat::R8_U_Int:
    case ElementFormat::R8_S_Int:
    case ElementFormat::R8_U_Norm:
    case ElementFormat::R8_S_Norm:
        return 1;
    case ElementFormat::R16_U_Int:
    case ElementFormat::R16_S_Int:
    case ElementFormat::R16_U_Norm:
    case ElementFormat::R16_S_Norm:
    case ElementFormat::R16_Float:
        return 2;
    case ElementFormat::R32_U_Int:
    case ElementFormat::R32_S_Int:
    case ElementFormat::R32_Float:
        return 4;
    case ElementFormat::R8G8_U_Int:
    case ElementFormat::R8G8_S_Int:
    case ElementFormat::R8G8_U_Norm:
    case ElementFormat::R8G8_S_Norm:
        return 2 * 1;
    case ElementFormat::R16G16_U_Int:
    case ElementFormat::R16G16_S_Int:
    case ElementFormat::R16G16_U_Norm:
    case ElementFormat::R16G16_S_Norm:
    case ElementFormat::R16G16_Float:
        return 2 * 2;
    case ElementFormat::R32G32_U_Int:
    case ElementFormat::R32G32_S_Int:
    case ElementFormat::R32G32_Float:
        return 2 * 4;
    case ElementFormat::R32G32B32_U_Int:
    case ElementFormat::R32G32B32_S_Int:
    case ElementFormat::R32G32B32_Float:
        return 3 * 4;
    case ElementFormat::R8G8B8A8_U_Int:
    case ElementFormat::R8G8B8A8_S_Int:
    case ElementFormat::R8G8B8A8_U_Norm:
    case ElementFormat::R8G8B8A8_U_Norm_sRGB:
    case ElementFormat::R8G8B8A8_S_Norm:
    case ElementFormat::B8G8R8A8_U_Norm:
    case ElementFormat::B8G8R8A8_U_Norm_sRGB:
        return 4 * 1;
    case ElementFormat::R16G16B16A16_U_Int:
    case ElementFormat::R16G16B16A16_S_Int:
    case ElementFormat::R16G16B16A16_U_Norm:
    case ElementFormat::R16G16B16A16_S_Norm:
    case ElementFormat::R16G16B16A16_Float:
        return 4 * 2;
    case ElementFormat::R32G32B32A32_U_Int:
    case ElementFormat::R32G32B32A32_S_Int:
    case ElementFormat::R32G32B32A32_Float:
        return 4 * 4;
    case ElementFormat::B5G6R5_U_norm:
    case ElementFormat::B5G5R5A1_U_Norm:
    case ElementFormat::B4G4R4A4_U_Norm:
        return 2;
    case ElementFormat::R11G11B10_Float:
    case ElementFormat::R10G10B10A2_U_Norm:
    case ElementFormat::R10G10B10A2_U_Int:
    case ElementFormat::R9G9B9E5_Float:
        return 4;
    case ElementFormat::BC1_U_Norm:
    case ElementFormat::BC1_U_Norm_sRGB:
    case ElementFormat::BC4_U_Norm:
    case ElementFormat::BC4_S_Norm:
        return 8;
    case ElementFormat::BC2_U_Norm:
    case ElementFormat::BC2_U_Norm_sRGB:
    case ElementFormat::BC3_U_Norm:
    case ElementFormat::BC3_U_Norm_sRGB:
    case ElementFormat::BC5_U_Norm:
    case ElementFormat::BC5_S_Norm:
    case ElementFormat::BC6H_U_Float:
    case ElementFormat::BC6H_S_Float:
    case ElementFormat::BC7_U_Norm:
    case ElementFormat::BC7_U_Norm_sRGB:
        return 16;
    }

    return 0;
}

} // namespace Renderer
} // namespace NFE
