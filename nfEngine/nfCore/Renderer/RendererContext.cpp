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

using namespace Common;

RenderContext::RenderContext()
{
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();

    commandRecorderGeometry = renderer->GetDevice()->CreateCommandRecorder();
    commandRecorderShadows = renderer->GetDevice()->CreateCommandRecorder();
    commandRecorderLights = renderer->GetDevice()->CreateCommandRecorder();
    commandRecorderDebug = renderer->GetDevice()->CreateCommandRecorder();
    commandRecorderOnScreen = renderer->GetDevice()->CreateCommandRecorder();

    geometryContext = MakeUniquePtr<GeometryRendererContext>(commandRecorderGeometry);
    shadowsContext = MakeUniquePtr<GeometryRendererContext>(commandRecorderShadows);
    lightsContext = MakeUniquePtr<LightsRendererContext>(commandRecorderLights);
    debugContext = MakeUniquePtr<DebugRendererContext>(commandRecorderDebug);
    postProcessContext = MakeUniquePtr<PostProcessRendererContext>(commandRecorderOnScreen);
    guiContext = MakeUniquePtr<GuiRendererContext>(commandRecorderOnScreen);
}

} // namespace Renderer
} // namespace NFE
