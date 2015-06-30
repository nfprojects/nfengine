/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Debug Renderer's context
 */

#pragma once

#include "../../nfCommon/Math/Math.hpp"

namespace NFE {
namespace Renderer {

using namespace Math;

enum class DebugPolyType
{
    Point = 0,
    Line,
    Triangle,
    TriangleTex,
    Mesh,
};

struct DebugVertex
{
    Float3 pos;
    uint32 color;
    Float2 texCoord;
};

typedef uint16 DebugIndexType;

/**
 * Debug renderer's per context data
 */
struct DebugRendererContext
{
    // RendererTextureD3D11* texture;
    DebugPolyType polyType;
    std::unique_ptr<DebugVertex[]> mVertices;
    std::unique_ptr<DebugIndexType[]> mIndicies;
    size_t mQueuedVertices;
    size_t mQueuedIndicies;

    DebugRendererContext();
};

} // namespace Renderer
} // namespace NFE
