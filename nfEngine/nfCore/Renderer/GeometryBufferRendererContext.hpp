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
    const RendererMaterial* currMaterial;
    std::unique_ptr<InstanceData[]> instanceData;

    GBufferRendererContext();
};

} // namespace Renderer
} // namespace NFE
