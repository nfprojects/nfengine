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
struct NFE_ALIGN(32) PostProcessRendererContext : Common::Aligned<32>
{
    CommandRecorderPtr commandRecorder;

    Math::Random random;

    NFE_INLINE PostProcessRendererContext(const CommandRecorderPtr& commandRecorder)
        : commandRecorder(commandRecorder)
    {
    }
};

} // namespace Renderer
} // namespace NFE
