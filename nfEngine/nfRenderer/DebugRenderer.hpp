/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Debug Renderer
 */

#pragma once

#include "nfRenderer.hpp"
#include "RendererModule.hpp"
#include "DebugRendererContext.hpp"


namespace NFE {
namespace Renderer {

using namespace Math;

/**
 * Renderer module capable of rendering debug shapes, such as: boxes,
 * spheres, lines, etc.
 */
class DebugRenderer : public RendererModule<DebugRenderer, DebugRendererContext>
{
    VertexLayoutPtr mVertexLayout;
    VertexLayoutPtr mMeshVertexLayout;
    MultiPipelineState mLinesPipelineState;
    MultiPipelineState mTrianglesPipelineState;
    MultiPipelineState mMeshPipelineState;

    BufferPtr mConstantBuffer;
    BufferPtr mPerMeshConstantBuffer;
    BufferPtr mVertexBuffer;
    BufferPtr mIndexBuffer;

    // TODO: material data (textures)
    ResourceBindingLayoutPtr mResBindingLayout;

    uint32 mIsMeshMacroId;
    uint32 mUseTextureMacroId;

    bool CreateResourceBindingLayouts();
    void SetMeshMaterial(DebugRendererContext* context, const Resource::Material* material);
    void Flush(DebugRendererContext* context);

public:
    DebugRenderer();

    void OnEnter(DebugRendererContext* context) override;
    void OnLeave(DebugRendererContext* context) override;

    void SetTarget(DebugRendererContext* context, const RenderTargetPtr& target);
    void SetCamera(DebugRendererContext* context, const Matrix4& viewMatrix, const Matrix4& projMatrix);
    void DrawLine(DebugRendererContext* context, const Vector4& A, const Vector4& B, const uint32 color);
    void DrawLine(DebugRendererContext* context, const Float3& A, const Float3& B, const uint32 color);
    void DrawBox(DebugRendererContext* context, const Box& box, const uint32 color);
    void DrawFilledBox(DebugRendererContext* context, const Box& box, const uint32 color);
    void DrawFrustum(DebugRendererContext* context, const Frustum& frustum, const uint32 color);
    void DrawMesh(DebugRendererContext* context, const Resource::Mesh* mesh, const Matrix4& matrix);
};

} // namespace Renderer
} // namespace NFE
