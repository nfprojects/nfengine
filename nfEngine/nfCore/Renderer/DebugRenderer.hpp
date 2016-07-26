/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Debug Renderer
 */

#pragma once

#include "../Prerequisites.hpp"
#include "RendererModule.hpp"
#include "DebugRendererContext.hpp"
#include "../Resources/MultiPipelineState.hpp"
#include "nfCommon/Math/Box.hpp"
#include "nfCommon/Math/Frustum.hpp"

namespace NFE {
namespace Renderer {

using namespace Math;

/**
 * Renderer module capable of rendering debug shapes, such as: boxes,
 * spheres, lines, etc.
 */
class DebugRenderer : public RendererModule<DebugRenderer, DebugRendererContext>
{
    std::unique_ptr<IVertexLayout> mVertexLayout;
    std::unique_ptr<IVertexLayout> mMeshVertexLayout;
    Resource::MultiPipelineState mLinesPipelineState;
    Resource::MultiPipelineState mTrianglesPipelineState;
    Resource::MultiPipelineState mMeshPipelineState;

    std::unique_ptr<IBuffer> mConstantBuffer;
    std::unique_ptr<IBuffer> mPerMeshConstantBuffer;
    std::unique_ptr<IBuffer> mVertexBuffer;
    std::unique_ptr<IBuffer> mIndexBuffer;

    // TODO: material data (textures)
    std::unique_ptr<IResourceBindingLayout> mResBindingLayout;

    uint32 mIsMeshMacroId;
    uint32 mUseTextureMacroId;

    bool CreateResourceBindingLayouts();
    void SetMeshMaterial(DebugRendererContext* context, const Resource::Material* material);
    void Flush(DebugRendererContext* context);

public:
    DebugRenderer();

    void OnEnter(DebugRendererContext* context) override;
    void OnLeave(DebugRendererContext* context) override;

    void SetTarget(DebugRendererContext* context, IRenderTarget* target);
    void SetCamera(DebugRendererContext* context, const Matrix& viewMatrix, const Matrix& projMatrix);
    void DrawLine(DebugRendererContext* context, const Vector& A, const Vector& B, const uint32 color);
    void DrawLine(DebugRendererContext* context, const Float3& A, const Float3& B, const uint32 color);
    void DrawBox(DebugRendererContext* context, const Box& box, const uint32 color);
    void DrawFilledBox(DebugRendererContext* context, const Box& box, const uint32 color);
    void DrawFrustum(DebugRendererContext* context, const Frustum& frustum, const uint32 color);
    void DrawMesh(DebugRendererContext* context, const Resource::Mesh* mesh, const Matrix& matrix);
};

} // namespace Renderer
} // namespace NFE
