/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Renderer Context
 */

#pragma once

#include "Renderers/RendererInterface/CommandRecorder.hpp"

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
    std::unique_ptr<ICommandRecorder> commandRecorderDebug;
    std::unique_ptr<ICommandRecorder> commandRecorderGeometry;
    std::unique_ptr<ICommandRecorder> commandRecorderShadows;
    std::unique_ptr<ICommandRecorder> commandRecorderLights;
    std::unique_ptr<ICommandRecorder> commandRecorderOnScreen;

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
