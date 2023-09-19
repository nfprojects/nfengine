/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Types.hpp"
#include "MemoryBlock.hpp"

#include <cfloat>


namespace NFE {
namespace Renderer {

/**
 * Texture's usage flags. The texture usage must be determined upon creation.
 */
enum class TextureUsageFlag : uint8
{
    ReadonlyShaderResource  = NFE_FLAG(0),
    ShaderWritableResource  = NFE_FLAG(1),
    RenderTarget            = NFE_FLAG(2),
    DepthStencil            = NFE_FLAG(3),
};

DEFINE_ENUM_OPERATORS(TextureUsageFlag)

/**
 * Texture subresource's range specified when binding a texture.
 */
struct TextureView
{
    uint32 baseMip = 0;
    uint32 baseLayer = 0;
    uint32 numMips = UINT32_MAX;
    uint32 numLayers = UINT32_MAX;
};

NFE_INLINE bool operator == (const TextureView& lhs, const TextureView& rhs)
{
    return
        lhs.baseMip == rhs.baseMip && lhs.baseLayer == rhs.baseLayer &&
        lhs.numMips == rhs.numMips && lhs.numLayers == rhs.numLayers;
}

/**
 * Texture descriptor.
 */
struct TextureDesc : public CommonResourceDesc
{
    TextureType type;
    Format format;
    ResourceAccessMode mode;
    TextureUsageFlag usage; //< texture usage
    uint32 width;           //< texture width in texels
    uint32 height;          //< texture height in texels, used only for 2D and 3D textures
    uint32 depth;           //< texture depth in texels, used only for 3D textures
    uint32 layers;          //< number of texture layers, used only for 1D and 2D textures
    uint32 samplesNum;      //< when bigger than 1, the texture is multisampled
    uint32 mipmaps;         //< number of mipmap levels

    // optional
    float defaultColorClearValue[4];
    float defaultDepthClearValue;
    uint8 defaultStencilClearValue;

    TextureDesc()
        : type(TextureType::Texture2D)
        , format(Format::Unknown)
        , mode(ResourceAccessMode::Immutable)
        , usage(static_cast<TextureUsageFlag>(0))
        , width(1)
        , height(1)
        , depth(1)
        , layers(1)
        , samplesNum(1)
        , mipmaps(1)
        , defaultColorClearValue{0.0f, 0.0f, 0.0f, 1.0f}
        , defaultDepthClearValue(1.0f)
        , defaultStencilClearValue(0)
    {}
};

class ITexture
{
public:
    virtual ~ITexture() {}
};

/**
 * Texture sampler descriptor.
 */
struct SamplerDesc
{
    TextureMinFilter minFilter; //< minification filter
    TextureMagFilter magFilter; //< magnification filter
    TextureWrapMode wrapModeU;  //< coordinate wrapping for U axis
    TextureWrapMode wrapModeV;  //< coordinate wrapping for V axis
    TextureWrapMode wrapModeW;  //< coordinate wrapping for W axis
    int maxAnisotropy;          //< maximum anisotropy level
    float minMipmap;            //< minimal mipmap level
    float maxMipmap;            //< maximal mimpap level
    float mipmapBias;           //< mipmap level bias

    /**
     * If set to "true", texels are compared against a reference value
     * specified in a shader.
     */
    bool compare;
    CompareFunc compareFunc; //< comparison function, used only when @p compare is true

    /**
     * Optional 4-element array describing border color for TextureWrapMode::Border wrapping mode.
     */
    float* borderColor;

    /*
        TODO:
        * comparison function
        * border color
        * LoD settings
     */

    const char* debugName; //< optional debug name

    SamplerDesc()
        : minFilter(TextureMinFilter::NearestMipmapNearest)
        , magFilter(TextureMagFilter::Nearest)
        , wrapModeU(TextureWrapMode::Repeat)
        , wrapModeV(TextureWrapMode::Repeat)
        , wrapModeW(TextureWrapMode::Repeat)
        , maxAnisotropy(16)
        , minMipmap(FLT_MIN)
        , maxMipmap(FLT_MAX)
        , mipmapBias(0.0f)
        , compare(false)
        , compareFunc(CompareFunc::Pass)
        , borderColor(nullptr)
        , debugName(nullptr)
    {}
};

class ISampler
{
public:
    virtual ~ISampler() {}
};

} // namespace Renderer
} // namespace NFE
