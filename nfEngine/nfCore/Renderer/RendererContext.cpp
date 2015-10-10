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

namespace NFE {
namespace Renderer {


RenderContext::RenderContext()
    : mIsDeferred(true)
{
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    commandBuffer = renderer->GetDevice()->CreateCommandBuffer();
}

RenderContext::RenderContext(ICommandBuffer* commandBuffer)
    : mIsDeferred(false)
{
    this->commandBuffer = commandBuffer;
}

RenderContext::~RenderContext()
{
    // immediate command buffer must not be deleted - it's owned by IDevice
    if (mIsDeferred && commandBuffer != nullptr)
    {
        delete commandBuffer;
        commandBuffer = nullptr;
    }
}

} // namespace Renderer
} // namespace NFE
