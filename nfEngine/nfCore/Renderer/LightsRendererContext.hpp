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
    ICommandRecorder* commandRecorder;

    NFE_INLINE LightsRendererContext(ICommandRecorder* commandRecorder)
        : commandRecorder(commandRecorder)
    {
    }
};

} // namespace Renderer
} // namespace NFE
