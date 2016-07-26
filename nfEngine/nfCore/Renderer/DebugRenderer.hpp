/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Debug Renderer
 */

#pragma once

#include "../Prerequisites.hpp"
#include "RendererModule.hpp"
#include "HighLevelRenderer.hpp"
#include "DebugRendererContext.hpp"
#include "../Resources/MultiShaderProgram.hpp"
#include "nfCommon/Math/Box.hpp"
#include "nfCommon/Math/Frustum.hpp"

namespace NFE {
namespace Renderer {

using namespace Math;

/**
 * Renderer module capable of rendering debug shapes, such as: boxes,
 * spheres, lines, etc.
 */
class DebugRenderer : public RendererModule<DebugRenderer>
{
    Resource::MultiShaderProgram mShaderProgram;
    std::unique_ptr<IVertexLayout> mVertexLayout;
    std::unique_ptr<IVertexLayout> mMeshVertexLayout;
    std::unique_ptr<IPipelineState> mLinesPipelineState;
    std::unique_ptr<IPipelineState> mTrianglesPipelineState;
    std::unique_ptr<IPipelineState> mMeshPipelineState;

    std::unique_ptr<IBuffer> mConstantBuffer;
    std::unique_ptr<IBuffer> mPerMeshConstantBuffer;
    std::unique_ptr<IBuffer> mVertexBuffer;
    std::unique_ptr<IBuffer> mIndexBuffer;

    // TODO: material data (textures)
    std::unique_ptr<IResourceBindingLayout> mResBindingLayout;

    uint32 mIsMeshMacroId;
    uint32 mUseTextureMacroId;

    bool CreateResourceBindingLayouts();
    void SetMeshMaterial(RenderContext* context, const Resource::Material* material);
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
    void DrawMesh(RenderContext* context, const Resource::Mesh* mesh, const Matrix& matrix);
};

} // namespace Renderer
} // namespace NFE
