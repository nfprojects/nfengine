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
    TexturePtr texture;    //< target texture object
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
    unsigned int numTargets;      //< number of targets
    RenderTargetElement* targets; //< array of RenderTargetElement
    TexturePtr depthBuffer;        //< optional pointer to a depth buffer's texture
    const char* debugName;        //< optional debug name

    RenderTargetDesc()
        : numTargets(0)
        , targets(nullptr)
        , depthBuffer(nullptr)
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

using RenderTargetPtr = AtomicSharedPtr<IRenderTarget>;

} // namespace Renderer
} // namespace NFE
