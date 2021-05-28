/**
 * @file
 * @author  Witek902
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "RendererCommon.hpp"
#include "../../Common/Containers/SharedPtr.hpp"
#include "../../Common/Reflection/ReflectionEnumMacros.hpp"


namespace NFE {
namespace Renderer {

/// predeclarations
struct MemoryBlockDesc;
class IMemoryBlock;
class ITexture;
class IBuffer;
class ISampler;
class IBackbuffer;
class IResourceBindingSet;
class IResourceBindingLayout;
class IResourceBindingInstance;
class ICommandRecorder;
class ICommandQueue;
class ICommandList;
class IPipelineState;
class IComputePipelineState;
class IRenderTarget;
class IShader;
class IVertexLayout;
class IFence;

using CommandRecorderPtr = Common::SharedPtr<ICommandRecorder>;
using CommandQueuePtr = Common::SharedPtr<ICommandQueue>;
using CommandListPtr = Common::UniquePtr<ICommandList>;
using MemoryBlockPtr = Common::SharedPtr<IMemoryBlock>;
using TexturePtr = Common::SharedPtr<ITexture>;
using BufferPtr = Common::SharedPtr<IBuffer>;
using SamplerPtr = Common::SharedPtr<ISampler>;
using BackbufferPtr = Common::SharedPtr<IBackbuffer>;
using ResourceBindingSetPtr = Common::SharedPtr<IResourceBindingSet>;
using ResourceBindingLayoutPtr = Common::SharedPtr<IResourceBindingLayout>;
using ResourceBindingInstancePtr = Common::SharedPtr<IResourceBindingInstance>;
using PipelineStatePtr = Common::SharedPtr<IPipelineState>;
using ComputePipelineStatePtr = Common::SharedPtr<IComputePipelineState>;
using RenderTargetPtr = Common::SharedPtr<IRenderTarget>;
using ShaderPtr = Common::SharedPtr<IShader>;
using VertexLayoutPtr = Common::SharedPtr<IVertexLayout>;
using FencePtr = Common::SharedPtr<IFence>;

#define MAX_RENDER_TARGETS 8
#define MAX_MIPMAP_LEVELS 16

// number of "graphics" shader types
#define NFE_GRAPHICS_SHADER_TYPES_NUM 5


enum class CommandQueueType : uint8
{
    Graphics,
    Compute,
    Copy,

    Invalid,
    Max = Invalid,
};

enum class ResourceType : uint8
{
    Texture,
    Buffer,

    Max,
};

/**
 * Common description of renderer's resource (buffer or texture)
 */
struct CommonResourceDesc
{
    // memory block to allocate the buffer from
    // if not provided, renderer will automatically manage memory for the resource
    MemoryBlockPtr memoryBlock;
    uint64 memoryBlockOffset = 0u;

    // optional debug name
    const char* debugName = nullptr;
};


/**
 * GPU resources data format.
 */
enum class Format : uint8
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

    // depth formats
    Depth16,            // 16-bit depth buffer
    Depth24_Stencil8,   // 24-bit depth buffer + 8-bit stencil buffer
    Depth32,            // 32-bit depth buffer
    Depth32_Stencil8,   // 32-bit depth buffer + 8-bit stencil buffer

    Max
};

enum class TextureType : uint8
{
    Texture1D,
    Texture2D,
    Texture3D,
    TextureCube,
    Unknown
};

enum class BufferType : uint8
{
    Vertex,
    Index,
    Constant
};

enum class ShaderType : uint8
{
    Unknown = 0xFF,
    Vertex  = 0,
    Hull,       //< aka. "tessellation control"
    Domain,     //< aka. "tessellation evaluation"
    Geometry,
    Pixel,      //< aka. "fragment"
    Compute,
    All
};

/**
 * GPU resource access modes.
 */
enum class ResourceAccessMode : uint8
{
    Invalid,

    /**
     * GPU read-only resource, for example a material's texture.
     * The content can't be accessed by the CPU. Can be written to only once.
     * This is the preferred access mode, as it has the lowest overhead.
     */
    Immutable,

    /**
     * GPU read-write resource, for example a texture used as a render target or a static texture sampled in a shader.
     * The content can't be accessed by the CPU. Can be written to many times per frame.
     */
    GPUOnly,

    /**
     * GPU read-only resource, for example a constant buffer.
     * The content can be written by the CPU.
     * WARNING: Memory accesses must be properly synchornized as it's not double-buffered.
     */
    Upload,

    /**
     * GPU read-only resource, frequently written by CPU.
     * The content can be written by the CPU. Assumes the data will be written to every frame.
     * This mode uses no actual Resource/Buffer allocation. Instead, internal Ring Buffer is used
     * to write data.
     */
    Volatile,

    /**
     * Readback resource, for example a screenshot texture.
     * The content can't be accessed directly by the GPU (only via Copy operations).
     * The data can be read by the CPU.
     * WARNING: Memory accesses must be properly synchornized as it's not double-buffered.
     */
    Readback
};

/**
 * GPU resource mapping type.
 */
enum class MapType : uint8
{
    WriteOnly,
    ReadOnly,
    ReadWrite
};

/**
 * Data format in a index buffer.
 */
enum class IndexBufferFormat : uint8
{
    Uint16,
    Uint32
};

/**
 * Alpha blending functions.
 */
enum class BlendFunc : uint8
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
enum class BlendOp : uint8
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
enum class CullMode : uint8
{
    Disabled, //< disable backface culling
    CW,   //< cull clockwise faces (default)
    CCW,  //< cull counter-clockwise faces
};

/**
 * Polygon filling mode.
 */
enum class FillMode : uint8
{
    Solid,    //< fill triangles (default)
    Wireframe //< draw wireframe only
};

/*
 * Comparison function.
 */
enum class CompareFunc : uint8
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

enum class StencilOp : uint8
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

enum class PrimitiveType : uint8
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
 * Texture addressing mode for coordinates outside range [0.0, 1.0)
 */
enum class TextureWrapMode : uint8
{
    Repeat,
    Clamp,
    Mirror,
    Border
};

/**
 * Texture minification filter.
 */
enum class TextureMinFilter : uint8
{
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear
};

/**
 * Texture magnification filter.
 */
enum class TextureMagFilter : uint8
{
    Nearest,
    Linear
};


/**
 * Get format pixel channels.
 */
NFE_RENDERER_COMMON_API uint32 GetFormatChannels(const Format format);

/**
 * Get bytes per element.
 */
NFE_RENDERER_COMMON_API uint32 GetFormatSize(const Format format);

/**
 * Convert format to string.
 */
NFE_RENDERER_COMMON_API const char* GetFormatName(const Format format);


NFE_INLINE bool IsDepthFormat(const Format format)
{
    return format == Format::Depth16 || format == Format::Depth24_Stencil8 || format == Format::Depth32 || format == Format::Depth32_Stencil8;
}

NFE_INLINE bool IsStencilFormat(const Format format)
{
    return format == Format::Depth24_Stencil8 || format == Format::Depth32_Stencil8;
}

NFE_INLINE bool IsFormatBlockCompressed(const Format format)
{
    return
        format == Format::BC1_U_Norm || format == Format::BC1_U_Norm_sRGB ||
        format == Format::BC2_U_Norm || format == Format::BC2_U_Norm_sRGB ||
        format == Format::BC3_U_Norm || format == Format::BC3_U_Norm_sRGB ||
        format == Format::BC4_U_Norm || format == Format::BC4_S_Norm ||
        format == Format::BC5_U_Norm || format == Format::BC5_S_Norm ||
        format == Format::BC6H_U_Float || format == Format::BC6H_S_Float ||
        format == Format::BC7_U_Norm || format == Format::BC7_U_Norm_sRGB;
}

NFE_INLINE bool IsFormatSRGB(const Format format)
{
    return
        format == Format::R8G8B8A8_U_Norm_sRGB ||
        format == Format::B8G8R8A8_U_Norm_sRGB ||
        format == Format::BC1_U_Norm_sRGB ||
        format == Format::BC2_U_Norm_sRGB ||
        format == Format::BC3_U_Norm_sRGB ||
        format == Format::BC7_U_Norm_sRGB;
}


} // namespace Renderer
} // namespace NFE

NFE_DECLARE_ENUM_TYPE(NFE::Renderer::Format)
