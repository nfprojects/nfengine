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

    ICommandBuffer* commandBuffer;
    std::vector<InstanceData, Common::AlignedAllocator<InstanceData, 64>> instanceData;

    NFE_INLINE GeometryRendererContext(ICommandBuffer* commandBuffer)
        : commandBuffer(commandBuffer)
    {
        instanceData.resize(gMaxBufferedInstances);
    }
};

} // namespace Renderer
} // namespace NFE
