/**
 * @file
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
    TexturePtr texture;     //< target texture object
    uint32 level;           //< target mipmap level within the texture
    uint32 layer;           //< target layer (or slice for 3D textures) within the texture
    Format format;          //< texture format override (use "Unknown" to inherit from the texture)

    RenderTargetElement(const TexturePtr& texture = nullptr)
        : texture(texture)
        , level(0)
        , layer(0)
        , format(Format::Unknown)
    {}
};

/*
 * Render target description.
 */
struct RenderTargetDesc
{
    uint32 numTargets;                  //< number of targets
    const RenderTargetElement* targets; //< array of RenderTargetElement
    TexturePtr depthBuffer;             //< optional pointer to a depth buffer's texture
    const char* debugName;              //< optional debug name

    RenderTargetDesc()
        : numTargets(0)
        , targets(nullptr)
        , debugName(nullptr)
    {}
};

/**
 * Render Target interface.
 *
 * @details Render Target object defines where rendering result will be written.
 *          A Render Target object may contain multiple Render Target Elements
 *          (aka. MRT - Multiple Render Targets).
 */
class IRenderTarget
{
public:
    virtual ~IRenderTarget() {}

    virtual void GetDimensions(int& width, int& height) = 0;
    virtual bool Init(const RenderTargetDesc& desc) = 0;
};

} // namespace Renderer
} // namespace NFE
