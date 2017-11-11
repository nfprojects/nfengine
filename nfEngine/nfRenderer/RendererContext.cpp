/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Renderer Context
 */

#pragma once

#include "PCH.hpp"
#include "RendererContext.hpp"
#include "HighLevelRenderer.hpp"

#include "GeometryRendererContext.hpp"
#include "LightsRendererContext.hpp"
#include "DebugRendererContext.hpp"
#include "GuiRendererContext.hpp"
#include "PostProcessRendererContext.hpp"


namespace NFE {
namespace Renderer {


RenderContext::RenderContext(HighLevelRenderer& renderer)
{
    commandRecorderGeometry = renderer.GetDevice()->CreateCommandRecorder();
    commandRecorderShadows = renderer.GetDevice()->CreateCommandRecorder();
    commandRecorderLights = renderer.GetDevice()->CreateCommandRecorder();
    commandRecorderDebug = renderer.GetDevice()->CreateCommandRecorder();
    commandRecorderOnScreen = renderer.GetDevice()->CreateCommandRecorder();

    geometryContext = Common::MakeUniquePtr<GeometryRendererContext>(commandRecorderGeometry);
    shadowsContext = Common::MakeUniquePtr<GeometryRendererContext>(commandRecorderShadows);
    lightsContext = Common::MakeUniquePtr<LightsRendererContext>(commandRecorderLights);
    debugContext = Common::MakeUniquePtr<DebugRendererContext>(commandRecorderDebug);
    postProcessContext = Common::MakeUniquePtr<PostProcessRendererContext>(commandRecorderOnScreen);
    guiContext = Common::MakeUniquePtr<GuiRendererContext>(commandRecorderOnScreen);
}

} // namespace Renderer
} // namespace NFE
