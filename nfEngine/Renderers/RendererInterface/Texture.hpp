/**
 * @file    Texture.hpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Types.hpp"

namespace NFE {
namespace Renderer {

// allow binding a texture as a shader resource
#define NFE_RENDERER_TEXTURE_BIND_SHADER (1 << 0)

// allow binding a texture as a rendertarget
#define NFE_RENDERER_TEXTURE_BIND_RENDERTARGET (1 << 1)

// allow binding a texture as a depth buffer
#define NFE_RENDERER_TEXTURE_BIND_DEPTH (1 << 2)

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
    int texelSize;  //< number of color channels per texel
    int binding;    //< texture binding mode
    int width;      //< texture width in texels
    int height;     //< texture height in texels, used only for 2D and 3D textures
    int depth;      //< texture depth in texels, used only for 3D textures
    int layers;     //< number of texture layers, used only for 1D and 2D textures
    int samplesNum; //< when bigger than 1, the texture is multisampled
    int mipmaps;    //< number of mipmap levels

    /**
     * Array of structures describing initial texture data for each mipmap level.
     * Can be NULL.
     */
    TextureDataDesc* dataDesc;

    TextureDesc()
        : type(TextureType::Texture2D)
        , format(ElementFormat::Int_8)
        , texelSize(1)
        , binding(0)
        , width(1)
        , height(1)
        , depth(1)
        , layers(1)
        , samplesNum(1)
        , mipmaps(1)
        , dataDesc(nullptr)
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
    TextureMinFilter minFilter;
    TextureMagFilter magFilter;
    TextureWrapMode wrapModeU;
    TextureWrapMode wrapModeV;
    TextureWrapMode wrapModeW;
    int maxAnisotropy;

    /*
        TODO:
        * comparison function
        * border color
        * LoD settings
     */

    SamplerDesc()
        : minFilter(TextureMinFilter::Nearest)
        , magFilter(TextureMagFilter::Nearest)
        , wrapModeU(TextureWrapMode::Repeat)
        , wrapModeV(TextureWrapMode::Repeat)
        , wrapModeW(TextureWrapMode::Repeat)
        , maxAnisotropy(16)
    {}
};

class ISampler
{
public:
    virtual ~ISampler() {}
};

} // namespace Renderer
} // namespace NFE
