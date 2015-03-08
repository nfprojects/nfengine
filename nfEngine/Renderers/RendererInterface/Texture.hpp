/**
 * @file    Texture.hpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Types.hpp"

namespace NFE {
namespace Renderer {

struct TextureDataDesc
{
    const void* data; //< pointer to the texture data
    size_t lineSize;  //< size of one line in bytes
    size_t sliceSize; //< size of one slice in bytes
};

struct TextureDesc
{
    TextureType type;
    ElementFormat format;
    int texelSize;

    int width;
    int height; //< used only for 2D and 3D textures
    int depth;  //< used only for 3D textures
    int layers; //< used only for 1D and 2D textures
    int mipmaps;

    /**
     * Array of structures describing initial texture data for each mipmap level.
     * Can be NULL.
     */
    TextureDataDesc* dataDesc;

    TextureDesc()
        : type(TextureType::Texture2D)
        , format(ElementFormat::Int_8)
        , texelSize(1)
        , width(1)
        , height(1)
        , depth(1)
        , layers(1)
        , mipmaps(1)
        , dataDesc(nullptr)
    {}
};

class ITexture
{
public:
    virtual ~ITexture() {}
};

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
    virtual ~ISampler()
    {
    }
};

} // namespace Renderer
} // namespace NFE
