/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Debug Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "HighLevelRenderer.hpp"
#include "RendererResources.hpp"
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

struct NFE_ALIGN16 DebugStateDesc
{
    Matrix viewMatrix;
    Matrix projMatrix;
};

struct NFE_ALIGN16 DebugCBuffer
{
    Matrix viewMatrix;
    Matrix projMatrix;
};

struct DebugVertex
{
    Float3 pos;
    uint32 color;
    Float2 texCoord;
};

// debug renderer per context data
struct DebugRendererData
{
    // RendererTextureD3D11* texture;
    DebugPolyType polyType;
    uint32* indexQueue;
    uint32 indexQueueSize;
    DebugVertex* vertexQueue;
    uint32 vertexQueueSize;
};

class DebugRenderer : public RendererModule<DebugRenderer>
{
public:
    void SetTarget(RenderContext* context, IRenderTarget* target);
    void SetCamera(RenderContext* context, const Matrix& viewMatrix,
                   const Matrix& projMatrix); // TODO: CameraRenderDesc
    void DrawLine(RenderContext* context, const Vector& A, const Vector& B, const uint32 color);
    void DrawLine(RenderContext* context, const Float3& A, const Float3& B, const uint32 color);
    void DrawBox(RenderContext* context, const Box& box, const uint32 color);
    void DrawFilledBox(RenderContext* context, const Box& box, const uint32 color);
    void DrawFrustum(RenderContext* context, const Frustum& frustum, const uint32 color);
    void DrawQuad(RenderContext *context, const Vector& pos, ITexture* texture, const uint32 color);
};

} // namespace Renderer
} // namespace NFE
