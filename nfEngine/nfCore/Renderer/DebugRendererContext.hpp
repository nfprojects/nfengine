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
    DebugRendererMode mode;
    PrimitiveType polyType;
    std::unique_ptr<DebugVertex[]> vertices;
    std::unique_ptr<DebugIndexType[]> indicies;
    size_t queuedVertices;
    size_t queuedIndicies;

    DebugRendererContext();
};

} // namespace Renderer
} // namespace NFE
