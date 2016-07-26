/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Renderer Context
 */

#pragma once

#include "Renderers/RendererInterface/CommandBuffer.hpp"

namespace NFE {
namespace Renderer {

struct GeometryRendererContext;
struct GeometryRendererContext;
struct LightsRendererContext;
struct PostProcessRendererContext;
struct DebugRendererContext;
struct GuiRendererContext;

class RenderContext
{
public:
    // low-level API command buffers for each pass
    // TODO: this can take too much driver and GPU memory...
    std::unique_ptr<ICommandBuffer> commandBufferDebug;
    std::unique_ptr<ICommandBuffer> commandBufferGeometry;
    std::unique_ptr<ICommandBuffer> commandBufferShadows;
    std::unique_ptr<ICommandBuffer> commandBufferLights;
    std::unique_ptr<ICommandBuffer> commandBufferOnScreen;

    std::unique_ptr<GeometryRendererContext> geometryContext;
    std::unique_ptr<GeometryRendererContext> shadowsContext;
    std::unique_ptr<LightsRendererContext> lightsContext;
    std::unique_ptr<PostProcessRendererContext> postProcessContext;
    std::unique_ptr<DebugRendererContext> debugContext;
    std::unique_ptr<GuiRendererContext> guiContext;

    RenderContext();
};

} // namespace Renderer
} // namespace NFE
