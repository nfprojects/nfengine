/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Renderer Context
 */

#pragma once

#include "RenderCommand.hpp"
#include "HighLevelRenderer.hpp"
#include "DebugRendererContext.hpp"

namespace NFE {
namespace Renderer {

#define MAX_BUFFERED_INSTANCES (8192)

class RenderContext
{
public:
    ICommandBuffer* commandBuffer; // low-level API command buffer

    RenderContext();
    RenderContext(ICommandBuffer* commandBuffer);

    /*
        TODO:

        Think of a way of adding new renderers' context data easly
        (without including renderer's header in this file.
     */
    DebugRendererContext debugContext;


    void Begin();
    void End();

    /*
        Execute deferred context pCtx.
        Setting 'saveState' to true restores previous context state after execution
    */
    bool Execute(RenderContext* context, bool saveState = false);
};

} // namespace Renderer
} // namespace NFE
