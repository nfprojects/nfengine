/**
 * @file    RenderTarget.hpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Texture.hpp"
#include "Types.hpp"

namespace NFE {
namespace Renderer {

/**
 * Description of a single render target texture.
 */
struct RenderTargetElement
{
    ITexture* texture;    //< target texture object
    int level;            //< target mipmap level within the texture
    int layer;            //< target layer (or slice for 3D textures) within the texture
    ElementFormat format; //< texture format override (use "Unknown" to inherit from the texture)

    RenderTargetElement()
        : texture(nullptr)
        , level(0)
        , layer(0)
        , format(ElementFormat::Unknown)
    {}
};

/*
 * Render target description.
 */
struct RenderTargetDesc
{
    unsigned int numTargets;          //< number of targets
    RenderTargetElement* targets;     //< array of RenderTargetElement
    RenderTargetElement* depthBuffer; //< optional pointer to a depth buffer description

    RenderTargetDesc()
        : numTargets(0)
        , targets(nullptr)
        , depthBuffer(nullptr)
    {}
};

class IRenderTarget
{
public:
    virtual ~IRenderTarget() {}

    virtual bool Init(const RenderTargetDesc& desc) = 0;
};

} // namespace Renderer
} // namespace NFE
