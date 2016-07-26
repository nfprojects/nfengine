/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of PostProcess Renderer's context
 */

#pragma once

#include "RenderCommand.hpp"
#include "nfCommon/Math/Random.hpp"


namespace NFE {
namespace Renderer {

/**
 * PostProcess Renderer's per context data
 */
struct PostProcessRendererContext
{
    ICommandBuffer* commandBuffer;

    Math::Random random;

    NFE_INLINE PostProcessRendererContext(ICommandBuffer* commandBuffer)
        : commandBuffer(commandBuffer)
    {
    }
};

} // namespace Renderer
} // namespace NFE
