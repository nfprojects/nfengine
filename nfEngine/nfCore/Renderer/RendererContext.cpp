/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Renderer Context
 */

#pragma once

#include "PCH.hpp"
#include "RendererContext.hpp"
#include "HighLevelRenderer.hpp"
#include "../Engine.hpp"

#include "GeometryRendererContext.hpp"
#include "LightsRendererContext.hpp"
#include "DebugRendererContext.hpp"
#include "GuiRendererContext.hpp"
#include "PostProcessRendererContext.hpp"


namespace NFE {
namespace Renderer {


RenderContext::RenderContext()
{
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();

    commandRecorderGeometry = renderer->GetDevice()->CreateCommandRecorder();
    commandRecorderShadows = renderer->GetDevice()->CreateCommandRecorder();
    commandRecorderLights = renderer->GetDevice()->CreateCommandRecorder();
    commandRecorderDebug = renderer->GetDevice()->CreateCommandRecorder();
    commandRecorderOnScreen = renderer->GetDevice()->CreateCommandRecorder();

    geometryContext = std::make_unique<GeometryRendererContext>(commandRecorderGeometry);
    shadowsContext = std::make_unique<GeometryRendererContext>(commandRecorderShadows);
    lightsContext = std::make_unique<LightsRendererContext>(commandRecorderLights);
    debugContext = std::make_unique<DebugRendererContext>(commandRecorderDebug);
    postProcessContext = std::make_unique<PostProcessRendererContext>(commandRecorderOnScreen);
    guiContext = std::make_unique<GuiRendererContext>(commandRecorderOnScreen);
}

} // namespace Renderer
} // namespace NFE
