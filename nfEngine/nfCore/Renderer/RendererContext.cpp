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

    commandRecorderGeometry.reset(renderer->GetDevice()->CreateCommandRecorder());
    commandRecorderShadows.reset(renderer->GetDevice()->CreateCommandRecorder());
    commandRecorderLights.reset(renderer->GetDevice()->CreateCommandRecorder());
    commandRecorderDebug.reset(renderer->GetDevice()->CreateCommandRecorder());
    commandRecorderOnScreen.reset(renderer->GetDevice()->CreateCommandRecorder());

    geometryContext = std::make_unique<GeometryRendererContext>(commandRecorderGeometry.get());
    shadowsContext = std::make_unique<GeometryRendererContext>(commandRecorderShadows.get());
    lightsContext = std::make_unique<LightsRendererContext>(commandRecorderLights.get());
    debugContext = std::make_unique<DebugRendererContext>(commandRecorderDebug.get());
    postProcessContext = std::make_unique<PostProcessRendererContext>(commandRecorderOnScreen.get());
    guiContext = std::make_unique<GuiRendererContext>(commandRecorderOnScreen.get());
}

} // namespace Renderer
} // namespace NFE
