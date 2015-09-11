/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Geometry Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "HighLevelRenderer.hpp"
#include "RenderCommand.hpp"
#include "GeometryRendererContext.hpp"
#include "RendererResources.hpp"
#include "Multishader.hpp"

namespace NFE {
namespace Renderer {

class GeometryRenderer : public RendererModule<GeometryRenderer>
{
    Multishader mVertexShader;
    Multishader mPixelShader;

    std::unique_ptr<IVertexLayout> mVertexLayout;
    std::unique_ptr<IRasterizerState> mRasterizerState;
    std::unique_ptr<IBuffer> mInstancesVertexBuffer;
    std::unique_ptr<IBuffer> mMaterialCBuffer;
    std::unique_ptr<IBuffer> mGlobalCBuffer;

    uint32 mUseMotionBlurMacroVS;
    uint32 mUseMotionBlurMacroPS;

public:
    GeometryRenderer();

    void OnEnter(RenderContext* context);
    void OnLeave(RenderContext* context);

    void SetUp(RenderContext *context, GeometryBuffer* geometryBuffer);
    void SetCamera(RenderContext* context, const CameraRenderDesc* camera);
    void SetMaterial(RenderContext* context, const RendererMaterial* material);
    void Draw(RenderContext* context, const RenderCommandBuffer& buffer);
};

} // namespace Renderer
} // namespace NFE
