/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of Lights Renderer
 */

#pragma once

#include "../PCH.hpp"
#include "LightsRenderer.hpp"
#include "../Globals.hpp"

namespace NFE {
namespace Renderer {

// renderer modules instance definition
std::unique_ptr<LightsRenderer> LightsRenderer::mPtr;


LightsRenderer::LightsRenderer()
{
    IDevice* device = gRenderer->GetDevice();

    mFullscreenQuadVS.Load("FullScreenQuadVS");
    mAmbientLightPS.Load("AmbientLightPS");

    /// create vertex layout
    VertexLayoutElement vertexLayoutElements[] =
    {
        { ElementFormat::Float_32, 3, 0, 0, false, 0 }, // position
    };
    VertexLayoutDesc vertexLayoutDesc;
    vertexLayoutDesc.elements = vertexLayoutElements;
    vertexLayoutDesc.numElements = 1;
    vertexLayoutDesc.vertexShader = mFullscreenQuadVS.GetShader(nullptr);
    vertexLayoutDesc.debugName = "LightsRenderer::mVertexLayout";
    mVertexLayout.reset(device->CreateVertexLayout(vertexLayoutDesc));


    Float3 quadVertices[] =
    {
        // triangle 0
        Float3(-1.0f, -1.0f, 0.0f),
        Float3( 1.0f, -1.0f, 0.0f),
        Float3( 1.0f,  1.0f, 0.0f),
        // triangle 1
        Float3(-1.0f, -1.0f, 0.0f),
        Float3( 1.0f,  1.0f, 0.0f),
        Float3(-1.0f,  1.0f, 0.0f),
    };


    BufferDesc bufferDesc;
    bufferDesc.access = BufferAccess::GPU_ReadOnly;
    bufferDesc.size = sizeof(quadVertices);
    bufferDesc.type = BufferType::Vertex;
    bufferDesc.initialData = quadVertices;
    bufferDesc.debugName = "LightsRenderer::mFullscreenQuadVB";
    mFullscreenQuadVB.reset(device->CreateBuffer(bufferDesc));
}

void LightsRenderer::OnEnter(RenderContext* context)
{
    context->commandBuffer->BeginDebugGroup("Lights Renderer stage");

    context->commandBuffer->SetShader(mFullscreenQuadVS.GetShader(nullptr));
    context->commandBuffer->SetVertexLayout(mVertexLayout.get());
}

void LightsRenderer::OnLeave(RenderContext* context)
{
    // TODO: allow "NULL" in the array
    ITexture* nullTextures[] = { gRenderer->GetDefaultDiffuseTexture(),
        gRenderer->GetDefaultDiffuseTexture(),
        gRenderer->GetDefaultDiffuseTexture(),
        gRenderer->GetDefaultDiffuseTexture(),
        gRenderer->GetDefaultDiffuseTexture(),
        gRenderer->GetDefaultDiffuseTexture() };
    context->commandBuffer->SetTextures(nullTextures, 6, ShaderType::Pixel);

    context->commandBuffer->EndDebugGroup();
}

void LightsRenderer::SetUp(RenderContext* context, IRenderTarget* target, GeometryBuffer *gbuffer,
                           const CameraRenderDesc* camera)
{
    context->commandBuffer->SetRenderTarget(target);

    /// bind gbuffer to pixel shader
    ITexture* textures[] = { gbuffer->mTextures[0].get(),
                             gbuffer->mTextures[1].get(),
                             gbuffer->mTextures[2].get(),
                             gbuffer->mTextures[3].get() };
    context->commandBuffer->SetTextures(textures, 4, ShaderType::Pixel);

    IBuffer* buffers[] = { mFullscreenQuadVB.get() };
    int strides[] = { sizeof(Float3) };
    int offsets[] = { 0 };
    context->commandBuffer->SetVertexBuffers(1, buffers, strides, offsets);

    context->commandBuffer->SetShader(mAmbientLightPS.GetShader(nullptr));
    
    context->commandBuffer->Draw(PrimitiveType::Triangles, 6);
}

void LightsRenderer::DrawAmbientLight(RenderContext* context, const Vector& ambientLightColor,
                                      const Vector& backgroundColor)
{
}

void LightsRenderer::TileBasedPass(RenderContext* context, uint32 lightsCount,
                                   const TileOmniLightDesc* lights)
{
}

void LightsRenderer::DrawOmniLight(RenderContext* context, const Vector& pos, float radius,
                                   const Vector& color, ShadowMap* shadowMap)
{
}

void LightsRenderer::DrawSpotLight(RenderContext* context, const SpotLightProperties& prop,
                                   const Frustum& frustum, ShadowMap* shadowMap,
                                   ITexture* lightMap)
{
}

void LightsRenderer::DrawDirLight(RenderContext* context, const DirLightProperties& prop,
                                  ShadowMap* shadowMap)
{
}

void LightsRenderer::DrawFog(RenderContext* context)
{
}

} // namespace Renderer
} // namespace NFE
