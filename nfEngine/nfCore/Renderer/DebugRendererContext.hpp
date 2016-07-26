/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Debug Renderer's context
 */

#pragma once

#include "nfCommon/Math/Math.hpp"

namespace NFE {
namespace Renderer {

using namespace Math;

struct DebugVertex
{
    Float3 pos;
    uint32 color;
    Float2 texCoord;
};

enum class DebugRendererMode
{
    Unknown,
    Simple,
    Meshes
};

typedef uint16 DebugIndexType;

/**
 * Debug renderer's per context data
 */
struct DebugRendererContext
{
    static const size_t gVertexBufferSize;
    static const size_t gIndexBufferSize;

    ICommandBuffer* commandBuffer;
    size_t queuedVertices;
    size_t queuedIndicies;
    DebugRendererMode mode;
    PrimitiveType polyType;
    std::unique_ptr<DebugVertex[]> vertices;
    std::unique_ptr<DebugIndexType[]> indicies;

    NFE_INLINE DebugRendererContext(ICommandBuffer* commandBuffer)
        : commandBuffer(commandBuffer)
        , queuedVertices(0)
        , queuedIndicies(0)
        , polyType(PrimitiveType::Lines)
        , mode(DebugRendererMode::Unknown)
    {
        /// allocate buffers for verticies and indicies
        vertices.reset(new DebugVertex[gVertexBufferSize]);
        indicies.reset(new DebugIndexType[gIndexBufferSize]);
    }
};

} // namespace Renderer
} // namespace NFE
