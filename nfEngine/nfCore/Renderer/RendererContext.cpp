/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Renderer Context
 */

#pragma once

#include "../PCH.hpp"
#include "RendererContext.hpp"
#include "HighLevelRenderer.hpp"
#include "../Engine.hpp"

namespace NFE {
namespace Renderer {


RenderContext::RenderContext()
{
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    commandBuffer = renderer->GetDevice()->CreateCommandBuffer();
}

RenderContext::RenderContext(ICommandBuffer* commandBuffer)
{
    this->commandBuffer = commandBuffer;
}

} // namespace Renderer
} // namespace NFE
