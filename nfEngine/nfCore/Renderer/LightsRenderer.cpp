/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of Lights Renderer
 */

#pragma once

#include "../PCH.hpp"
#include "LightsRenderer.hpp"

namespace NFE {
namespace Renderer {

namespace {

struct NFE_ALIGN16 LightsGlobalCBuffer
{
    Matrix cameraMatrix;
    Matrix viewMatrix;
    Matrix projMatrix;
    Vector viewportResInv;
    Vector screenScale;
};

struct AmbientLightCBuffer
{
    Float4 ambientLight;
    Float4 backgroundColor;
};

struct NFE_ALIGN16 OmniLightCBuffer
{
    Vector position;
    Vector radius;
    Vector color;
};

} // namespace

// renderer modules instance definition
std::unique_ptr<LightsRenderer> LightsRenderer::mPtr;


LightsRenderer::LightsRenderer()
{
    IDevice* device = mRenderer->GetDevice();

    mFullscreenQuadVS.Load("FullScreenQuadVS");
    mAmbientLightPS.Load("AmbientLightPS");
    mOmniLightVS.Load("OmniLightVS");
    mOmniLightPS.Load("OmniLightPS");
    mSpotLightVS.Load("SpotLightVS");
    mSpotLightPS.Load("SpotLightPS");

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

    /**
     * Constants used to generate regular icosahedron vertices coordinates:
     * phi   - golden ratio = (sqrt(5) + 1) / 2
     * scale - scaling factor. Scale equal to 1.0 would generate icosahedron with edge
     *         length a = 2.0. We need icosahedron for wihich inscribed sphere has
     *         radius r = 1.0. The equation for r is: r = sqrt(3) / 12 * (3 + sqrt(5)) * a,
     *         so we need to scale the shape by the factor of:
     *             scale = 1 / (sqrt(3) / 12 * (3 + sqrt(5)) * 2) = 0.661584538...
     */
    const float phi = 1.6180345f;
    const float scale = 0.661584538f;

    // TODO: use different shapes for omni lights. For example, more detailed for
    // the nearest lights (icosahedron or even more complex shapes) and simple (cube) for
    // distant lights.

    Float3 vertices[] =
    {
        // full-screen quad (for ambient light)
        Float3(-1.0f, -1.0f, 0.0f),
        Float3( 1.0f, -1.0f, 0.0f),
        Float3( 1.0f,  1.0f, 0.0f),
        Float3(-1.0f,  1.0f, 0.0f),

        // regular icosahedron (for omni lights)
        Float3(0.0f, -phi * scale, -scale),
        Float3(0.0f,  phi * scale, -scale),
        Float3(0.0f,  phi * scale,  scale),
        Float3(0.0f, -phi * scale,  scale),
        Float3(-scale, 0.0f, -phi * scale),
        Float3(-scale, 0.0f,  phi * scale),
        Float3( scale, 0.0f,  phi * scale),
        Float3( scale, 0.0f, -phi * scale),
        Float3(-phi * scale, -scale, 0.0f),
        Float3(-phi * scale,  scale, 0.0f),
        Float3( phi * scale,  scale, 0.0f),
        Float3( phi * scale, -scale, 0.0f),
    };

    uint16 indices[] =
    {
        // indices for full-screen quad
        0, 1, 2, 0, 2, 3,

        // indices for icosahedron
        2, 1, 10, 2, 10, 6, 2, 6, 5, 2, 5, 9, 2, 9, 1, // top cap
        0, 11, 7, 0, 7, 4, 0, 4, 8, 0, 8, 3, 0, 3, 11, // bottom cap
        9, 4, 1, 4, 7, 1, 7, 10, 1, 7, 11, 10, 11, 6, 10,
        6, 11, 3, 6, 3, 5, 5, 3, 8, 9, 5, 8, 9, 8, 4,
    };

    BufferDesc bufferDesc;
    bufferDesc.access = BufferAccess::GPU_ReadOnly;
    bufferDesc.size = sizeof(vertices);
    bufferDesc.type = BufferType::Vertex;
    bufferDesc.initialData = vertices;
    bufferDesc.debugName = "LightsRenderer::mVertexBuffer";
    mVertexBuffer.reset(device->CreateBuffer(bufferDesc));

    bufferDesc.access = BufferAccess::GPU_ReadOnly;
    bufferDesc.size = sizeof(indices);
    bufferDesc.type = BufferType::Index;
    bufferDesc.initialData = indices;
    bufferDesc.debugName = "LightsRenderer::mIndexBuffer";
    mIndexBuffer.reset(device->CreateBuffer(bufferDesc));

    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = sizeof(LightsGlobalCBuffer);
    bufferDesc.type = BufferType::Constant;
    bufferDesc.initialData = nullptr;
    bufferDesc.debugName = "LightsRenderer::mGlobalCBuffer";
    mGlobalCBuffer.reset(device->CreateBuffer(bufferDesc));

    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = sizeof(AmbientLightCBuffer);
    bufferDesc.type = BufferType::Constant;
    bufferDesc.initialData = nullptr;
    bufferDesc.debugName = "LightsRenderer::mAmbientLightCBuffer";
    mAmbientLightCBuffer.reset(device->CreateBuffer(bufferDesc));

    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = sizeof(OmniLightCBuffer);
    bufferDesc.type = BufferType::Constant;
    bufferDesc.initialData = nullptr;
    bufferDesc.debugName = "LightsRenderer::mOmniLightCBuffer";
    mOmniLightCBuffer.reset(device->CreateBuffer(bufferDesc));


    // depth state for light volumes rendering
    DepthStateDesc dsDesc;
    dsDesc.depthCompareFunc = CompareFunc::Grater;
    dsDesc.depthTestEnable = false;
    dsDesc.depthWriteEnable = false;
    dsDesc.debugName = "LightsRenderer::mLightsDepthState";
    mLightsDepthState.reset(device->CreateDepthState(dsDesc));

    // rasterizer state for light volumes rendering
    RasterizerStateDesc rsDesc;
    rsDesc.cullMode = CullMode::CCW;
    rsDesc.debugName = "LightsRenderer::mLightsRasterizerState";
    mLightsRasterizerState.reset(device->CreateRasterizerState(rsDesc));

    // blend state that enables additive alpha-blending
    BlendStateDesc bsDesc;
    bsDesc.rtDescs[0].enable = true;
    bsDesc.debugName = "LightsRenderer::mLightsBlendState";
    mLightsBlendState.reset(device->CreateBlendState(bsDesc));
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
    ITexture* nullTextures[] = { mRenderer->GetDefaultDiffuseTexture(),
        mRenderer->GetDefaultDiffuseTexture(),
        mRenderer->GetDefaultDiffuseTexture(),
        mRenderer->GetDefaultDiffuseTexture(),
        mRenderer->GetDefaultDiffuseTexture(),
        mRenderer->GetDefaultDiffuseTexture() };
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
        gbuffer->mTextures[3].get(),
        gbuffer->mDepthBuffer.get() };
    context->commandBuffer->SetTextures(textures, 5, ShaderType::Pixel);

    IBuffer* buffers[] = { mVertexBuffer.get() };
    int strides[] = { sizeof(Float3) };
    int offsets[] = { 0 };
    context->commandBuffer->SetVertexBuffers(1, buffers, strides, offsets);
    context->commandBuffer->SetIndexBuffer(mIndexBuffer.get(), IndexBufferFormat::Uint16);

    LightsGlobalCBuffer cbuffer;
    cbuffer.cameraMatrix = camera->matrix;
    cbuffer.projMatrix = camera->projMatrix;
    cbuffer.viewMatrix = camera->viewMatrix;
    cbuffer.viewportResInv = Vector(1.0f / gbuffer->GetWidth(), 1.0f / gbuffer->GetHeight());
    cbuffer.screenScale = camera->screenScale;
    context->commandBuffer->WriteBuffer(mGlobalCBuffer.get(), 0, sizeof(LightsGlobalCBuffer),
                                        &cbuffer);
}

void LightsRenderer::DrawAmbientLight(RenderContext* context, const Vector& ambientLightColor,
                                      const Vector& backgroundColor)
{
    context->commandBuffer->SetBlendState(mRenderer->GetDefaultBlendState());

    AmbientLightCBuffer cbuffer;
    VectorStore(ambientLightColor, &cbuffer.ambientLight);
    VectorStore(backgroundColor, &cbuffer.backgroundColor);
    context->commandBuffer->WriteBuffer(mAmbientLightCBuffer.get(), 0, sizeof(AmbientLightCBuffer),
                                        &cbuffer);

    IBuffer* cbuffers[] = { mAmbientLightCBuffer.get() };
    context->commandBuffer->SetShader(mAmbientLightPS.GetShader(nullptr));
    context->commandBuffer->SetConstantBuffers(cbuffers, 1, ShaderType::Pixel);

    context->commandBuffer->DrawIndexed(PrimitiveType::Triangles, 6);
}

void LightsRenderer::TileBasedPass(RenderContext* context, uint32 lightsCount,
                                   const TileOmniLightDesc* lights)
{
}

void LightsRenderer::DrawOmniLight(RenderContext* context, const Vector& pos, float radius,
                                   const Vector& color, ShadowMap* shadowMap)
{
    // TODO: use instancing to draw lights

    context->commandBuffer->SetDepthState(mLightsDepthState.get());
    context->commandBuffer->SetRasterizerState(mLightsRasterizerState.get());
    context->commandBuffer->SetBlendState(mLightsBlendState.get());

    context->commandBuffer->SetShader(mOmniLightVS.GetShader(nullptr));
    context->commandBuffer->SetShader(mOmniLightPS.GetShader(nullptr));

    IBuffer* cbuffers[] = { mGlobalCBuffer.get(), mOmniLightCBuffer.get() };
    context->commandBuffer->SetConstantBuffers(cbuffers, 2, ShaderType::Vertex);
    context->commandBuffer->SetConstantBuffers(cbuffers, 2, ShaderType::Pixel);

    OmniLightCBuffer cbuffer;
    cbuffer.position = pos;
    cbuffer.radius = VectorSplat(radius);
    cbuffer.color = color;
    context->commandBuffer->WriteBuffer(mOmniLightCBuffer.get(), 0, sizeof(OmniLightCBuffer),
                                        &cbuffer);

    context->commandBuffer->DrawIndexed(PrimitiveType::Triangles,
                                        20 * 3, // 20 triangles
                                        -1,     // no instancing
                                        6,      // ignore first 6 indices
                                        4);     // ignore first 4 vertices
}

void LightsRenderer::DrawSpotLight(RenderContext* context, const SpotLightProperties& prop,
                                   ShadowMap* shadowMap, ITexture* lightMap)
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
