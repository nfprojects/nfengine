/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Geometry Buffer Renderer's context
 */

#pragma once

#include "RenderCommand.hpp"

namespace NFE {
namespace Renderer {

using namespace Math;

/**
 * Geometry Buffer Renderer's per context data
 */
struct GBufferRendererContext
{
    std::vector<InstanceData, Common::AlignedAllocator<InstanceData, 64>> instanceData;

    GBufferRendererContext();

};

} // namespace Renderer
} // namespace NFE
