/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Types.hpp"

#include <cfloat>

namespace NFE {
namespace Renderer {

// allow binding a texture as a shader resource
#define NFE_RENDERER_TEXTURE_BIND_SHADER (1 << 0)

// allow binding a texture as a rendertarget
#define NFE_RENDERER_TEXTURE_BIND_RENDERTARGET (1 << 1)

// allow binding a texture as a depth buffer
#define NFE_RENDERER_TEXTURE_BIND_DEPTH (1 << 2)

// allow binding a texture as a writable shader resource
#define NFE_RENDERER_TEXTURE_BIND_SHADER_WRITABLE (1 << 3)

/**
 * Initial texture data descriptor.
 */
struct TextureDataDesc
{
    const void* data; //< pointer to the texture data
    size_t lineSize;  //< size of one line in bytes
    size_t sliceSize; //< size of one slice in bytes
};

/**
 * Texture descriptor.
 */
struct TextureDesc
{
    TextureType type;
    ElementFormat format;
    BufferMode mode;
    DepthBufferFormat depthBufferFormat; //< set when texture is used as depth buffer
    int binding;        //< texture binding mode
    uint32 width;       //< texture width in texels
    uint32 height;      //< texture height in texels, used only for 2D and 3D textures
    uint32 depth;       //< texture depth in texels, used only for 3D textures
    uint32 layers;      //< number of texture layers, used only for 1D and 2D textures
    uint32 samplesNum;  //< when bigger than 1, the texture is multisampled
    uint32 mipmaps;     //< number of mipmap levels

    /**
     * Array of structures describing initial texture data for each mipmap level.
     * Can be NULL.
     */
    TextureDataDesc* dataDesc;

    const char* debugName; //< optional debug name

    TextureDesc()
        : type(TextureType::Texture2D)
        , format(ElementFormat::R8G8B8A8_U_Norm)
        , mode(BufferMode::Static)
        , depthBufferFormat(DepthBufferFormat::Unknown)
        , binding(0)
        , width(1)
        , height(1)
        , depth(1)
        , layers(1)
        , samplesNum(1)
        , mipmaps(1)
        , dataDesc(nullptr)
        , debugName(nullptr)
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
