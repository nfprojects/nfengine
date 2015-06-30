/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Debug Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "HighLevelRenderer.hpp"
#include "DebugRendererContext.hpp"
#include "../../nfCommon/Math/Math.hpp"

namespace NFE {
namespace Renderer {

using namespace Math;

/**
 * Renderer module capable of rendering debug shapes, such as: boxes,
 * spheres, lines, etc.
 */
class DebugRenderer : public RendererModule<DebugRenderer>
{
    std::unique_ptr<IShader> mVertexShader;
    std::unique_ptr<IShader> mPixelShader;

    std::unique_ptr<IBuffer> mConstantBuffer;
    std::unique_ptr<IBuffer> mVertexBuffer;
    std::unique_ptr<IBuffer> mIndexBuffer;

    void Flush(RenderContext* context);

public:
    DebugRenderer();

    void OnEnter(RenderContext* context);
    void OnLeave(RenderContext* context);

    void SetTarget(RenderContext* context, IRenderTarget* target);
    void SetCamera(RenderContext* context, const Matrix& viewMatrix,
                   const Matrix& projMatrix);
    void DrawLine(RenderContext* context, const Vector& A, const Vector& B, const uint32 color);
    void DrawLine(RenderContext* context, const Float3& A, const Float3& B, const uint32 color);
    void DrawBox(RenderContext* context, const Box& box, const uint32 color);
    void DrawFilledBox(RenderContext* context, const Box& box, const uint32 color);
    void DrawFrustum(RenderContext* context, const Frustum& frustum, const uint32 color);
    void DrawQuad(RenderContext *context, const Vector& pos, ITexture* texture, const uint32 color);
};

} // namespace Renderer
} // namespace NFE
