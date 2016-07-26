/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Lights Renderer's context
 */

#pragma once

#include "RenderCommand.hpp"

namespace NFE {
namespace Renderer {

/**
 * Lights Renderer's per context data
 */
struct LightsRendererContext
{
    ICommandBuffer* commandBuffer;

    NFE_INLINE LightsRendererContext(ICommandBuffer* commandBuffer)
        : commandBuffer(commandBuffer)
    {
    }
};

} // namespace Renderer
} // namespace NFE
