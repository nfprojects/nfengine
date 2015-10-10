/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Renderer Context
 */

#pragma once

#include "RenderCommand.hpp"
#include "HighLevelRenderer.hpp"
#include "DebugRendererContext.hpp"
#include "GeometryRendererContext.hpp"
#include "GuiRendererContext.hpp"

namespace NFE {
namespace Renderer {

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
    GBufferRendererContext geometryBufferContext;
    GuiRendererContext guiContext;
};

} // namespace Renderer
} // namespace NFE
