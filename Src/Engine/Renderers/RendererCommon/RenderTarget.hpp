/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Texture.hpp"
#include "Types.hpp"
#include "../../Common/Containers/StaticArray.hpp"

namespace NFE {
namespace Renderer {

/**
 * Description of a single render target texture.
 */
struct RenderTargetElement
{
    TexturePtr texture;     //< target texture object
    uint16 layer;           //< target layer (or slice for 3D textures) within the texture
    uint8 level;            //< target mipmap level within the texture
    Format format;          //< texture format override (use "Unknown" to inherit from the texture)

    RenderTargetElement(const TexturePtr& texture = nullptr)
        : texture(texture)
        , layer(0u)
        , level(0u)
        , format(Format::Unknown)
    {}
};

/*
 * Render target description.
 */
struct RenderTargetDesc
{
    // array of RenderTargetElements
    Common::StaticArray<RenderTargetElement, MAX_RENDER_TARGETS> targets;

    // optional pointer to a depth buffer's texture
    TexturePtr depthBuffer;

    // optional debug name
    const char* debugName = nullptr;
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
