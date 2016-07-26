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

    commandBufferGeometry.reset(renderer->GetDevice()->CreateCommandBuffer());
    commandBufferShadows.reset(renderer->GetDevice()->CreateCommandBuffer());
    commandBufferLights.reset(renderer->GetDevice()->CreateCommandBuffer());
    commandBufferDebug.reset(renderer->GetDevice()->CreateCommandBuffer());
    commandBufferOnScreen.reset(renderer->GetDevice()->CreateCommandBuffer());

    geometryContext = std::make_unique<GeometryRendererContext>(commandBufferGeometry.get());
    shadowsContext = std::make_unique<GeometryRendererContext>(commandBufferShadows.get());
    lightsContext = std::make_unique<LightsRendererContext>(commandBufferLights.get());
    debugContext = std::make_unique<DebugRendererContext>(commandBufferDebug.get());
    postProcessContext = std::make_unique<PostProcessRendererContext>(commandBufferOnScreen.get());
    guiContext = std::make_unique<GuiRendererContext>(commandBufferOnScreen.get());
}

} // namespace Renderer
} // namespace NFE
