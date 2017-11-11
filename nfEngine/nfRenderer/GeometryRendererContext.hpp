/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Geometry Renderer's context
 */

#pragma once

#include "RenderCommand.hpp"

namespace NFE {
namespace Renderer {

using namespace Math;

/**
 * Geometry Buffer Renderer's per context data
 */
struct GeometryRendererContext
{
    static const size_t gMaxBufferedInstances;

    CommandRecorderPtr commandRecorder;
    std::vector<InstanceData, Common::AlignedAllocator<InstanceData, 64>> instanceData;

    NFE_INLINE GeometryRendererContext(const CommandRecorderPtr& commandRecorder)
        : commandRecorder(commandRecorder)
    {
        instanceData.resize(gMaxBufferedInstances);
    }
};

} // namespace Renderer
} // namespace NFE
