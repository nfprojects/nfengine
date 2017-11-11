/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Renderer Context
 */

#pragma once

#include "Renderers/RendererInterface/CommandRecorder.hpp"

#include "nfCommon/Containers/UniquePtr.hpp"

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
    CommandRecorderPtr commandRecorderDebug;
    CommandRecorderPtr commandRecorderGeometry;
    CommandRecorderPtr commandRecorderShadows;
    CommandRecorderPtr commandRecorderLights;
    CommandRecorderPtr commandRecorderOnScreen;

    Common::UniquePtr<GeometryRendererContext> geometryContext;
    Common::UniquePtr<GeometryRendererContext> shadowsContext;
    Common::UniquePtr<LightsRendererContext> lightsContext;
    Common::UniquePtr<PostProcessRendererContext> postProcessContext;
    Common::UniquePtr<DebugRendererContext> debugContext;
    Common::UniquePtr<GuiRendererContext> guiContext;

    explicit RenderContext(HighLevelRenderer& renderer);
};

} // namespace Renderer
} // namespace NFE
