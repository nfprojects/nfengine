/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of Lights Renderer
 */

#pragma once

#include "PCH.hpp"
#include "LightsRenderer.hpp"
#include "../../nfCommon/Logger.hpp"

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
    Vector shadowMapProps;
};

} // namespace

// renderer modules instance definition
std::unique_ptr<LightsRenderer> LightsRenderer::mPtr;


LightsRenderer::LightsRenderer()
{
    using namespace Resource;

    IDevice* device = mRenderer->GetDevice();

    mAmbientLightShaderProgram.Load("AmbientLight");

    mOmniLightShaderProgram.Load("OmniLight");
    mOmniLightUseShadowMap = mOmniLightShaderProgram.GetMacroByName("USE_SHADOW_MAP");

    mSpotLightShaderProgram.Load("SpotLight");
    mSpotLightUseLightMap = mSpotLightShaderProgram.GetMacroByName("USE_LIGHT_MAP");
    mSpotLightUseShadowMap = mSpotLightShaderProgram.GetMacroByName("USE_SHADOW_MAP");


    /// create vertex layout
    VertexLayoutElement vertexLayoutElements[] =
    {
        { ElementFormat::Float_32, 3, 0, 0, false, 0 }, // position
    };
    VertexLayoutDesc vertexLayoutDesc;
    vertexLayoutDesc.elements = vertexLayoutElements;
    vertexLayoutDesc.numElements = 1;
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

        // frustum vertices in screen-space (for spot light)
        Float3(-1.0f, -1.0f, -1.0f),
        Float3( 1.0f, -1.0f, -1.0f),
        Float3(-1.0f,  1.0f, -1.0f),
        Float3( 1.0f,  1.0f, -1.0f),
        Float3(-1.0f, -1.0f, 1.0f),
        Float3( 1.0f, -1.0f, 1.0f),
        Float3(-1.0f,  1.0f, 1.0f),
        Float3( 1.0f,  1.0f, 1.0f),
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

        // indices for frustum
        0, 1, 3,  0, 3, 2, // near side
        1, 5, 7,  1, 7, 3, // right side
        5, 6, 7,  5, 4, 6, // far side
        4, 0, 2,  4, 2, 6, // left side
        2, 3, 7,  2, 7, 6, // top side
        4, 1, 0,  4, 5, 1, // bottom side
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

    // constant buffers
    {
        bufferDesc.access = BufferAccess::CPU_Write;
        bufferDesc.type = BufferType::Constant;
        bufferDesc.initialData = nullptr;

        bufferDesc.size = sizeof(LightsGlobalCBuffer);
        bufferDesc.debugName = "LightsRenderer::mGlobalCBuffer";
        mGlobalCBuffer.reset(device->CreateBuffer(bufferDesc));

        bufferDesc.size = sizeof(AmbientLightCBuffer);
        bufferDesc.debugName = "LightsRenderer::mAmbientLightCBuffer";
        mAmbientLightCBuffer.reset(device->CreateBuffer(bufferDesc));

        bufferDesc.size = sizeof(OmniLightCBuffer);
        bufferDesc.debugName = "LightsRenderer::mOmniLightCBuffer";
        mOmniLightCBuffer.reset(device->CreateBuffer(bufferDesc));

        bufferDesc.size = sizeof(SpotLightProperties);
        bufferDesc.debugName = "LightsRenderer::mSpotLightCBuffer";
        mSpotLightCBuffer.reset(device->CreateBuffer(bufferDesc));
    }

    CreateResourceBindingLayouts();

    // binding instances for cbuffers
    {
        mOmniLightBindingInstance.reset(
            device->CreateResourceBindingInstance(mGlobalBindingSet.get()));
        if (mOmniLightBindingInstance)
        {
            mOmniLightBindingInstance->WriteCBufferView(0, mGlobalCBuffer.get());
            mOmniLightBindingInstance->WriteCBufferView(1, mOmniLightCBuffer.get());
        }

        mSpotLightBindingInstance.reset(
            device->CreateResourceBindingInstance(mGlobalBindingSet.get()));
        if (mSpotLightBindingInstance)
        {
            mSpotLightBindingInstance->WriteCBufferView(0, mGlobalCBuffer.get());
            mSpotLightBindingInstance->WriteCBufferView(1, mSpotLightCBuffer.get());
        }
    }

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.resBindingLayout = mResBindingLayout.get();
    pipelineStateDesc.vertexLayout = mVertexLayout.get();

    pipelineStateDesc.debugName = "LightsRenderer::mAmbientLightPipelineState";
    mAmbientLightPipelineState.reset(device->CreatePipelineState(pipelineStateDesc));

    // blend state that enables additive alpha-blending
    pipelineStateDesc.blendState.rtDescs[0].enable = true;
    // depth state for light volumes rendering
    pipelineStateDesc.depthState.depthCompareFunc = CompareFunc::Greater;
    pipelineStateDesc.depthState.depthTestEnable = true;
    pipelineStateDesc.depthState.depthWriteEnable = false;
    // rasterizer state for light volumes rendering
    pipelineStateDesc.raterizerState.cullMode = CullMode::CCW;
    pipelineStateDesc.debugName = "LightsRenderer::mLightVolumePipelineState";
    mLightVolumePipelineState.reset(device->CreatePipelineState(pipelineStateDesc));

    // shadow map sampler
    SamplerDesc samplerDesc;
    samplerDesc.magFilter = TextureMagFilter::Linear;
    samplerDesc.minFilter = TextureMinFilter::NearestMipmapLinear;
    samplerDesc.debugName = "LightsRenderer::mShadowMapSampler";
    samplerDesc.compare = true;
    samplerDesc.wrapModeU = samplerDesc.wrapModeV = TextureWrapMode::Clamp;
    samplerDesc.compareFunc = CompareFunc::LessEqual;
    mShadowMapSampler.reset(device->CreateSampler(samplerDesc));
}

bool LightsRenderer::CreateResourceBindingLayouts()
{
    IDevice* device = mRenderer->GetDevice();

    int globalCBufferSlot = mOmniLightShaderProgram.GetResourceSlotByName("Global");
    int lightParamsSlot = mOmniLightShaderProgram.GetResourceSlotByName("OmniLightProps");
    // TODO: verify other light types
    if (globalCBufferSlot < 0 || lightParamsSlot < 0)
    {
        LOG_ERROR("Invalid cbuffer slot");
        return false;
    }

    int gbufferTex0Slot = mOmniLightShaderProgram.GetResourceSlotByName("gGBufferTex0");
    int gbufferTex1Slot = mOmniLightShaderProgram.GetResourceSlotByName("gGBufferTex1");
    // not used in pixel shaders right now
    // int gbufferTex2Slot = mOmniLightShaderProgram.GetResourceSlotByName("gGBufferTex2");
    // int gbufferTex3Slot = mOmniLightShaderProgram.GetResourceSlotByName("gGBufferTex3");
    if (gbufferTex0Slot < 0 || gbufferTex1Slot < 0
        /* || gbufferTex2Slot < 0 || gbufferTex3Slot < 0 */ )
    {
        LOG_ERROR("Invalid gbuffer slot");
        return false;
    }

    int shadowMapSlot = mOmniLightShaderProgram.GetResourceSlotByName("gShadowMap");
    if (shadowMapSlot < 0)
    {
        LOG_ERROR("Invalid shadowMapSlot slot");
        return false;
    }

    // global binding set (cbuffers with camera and light properties)
    ResourceBindingDesc globalBindings[2] =
    {
        ResourceBindingDesc(ShaderResourceType::CBuffer, globalCBufferSlot),
        ResourceBindingDesc(ShaderResourceType::CBuffer, lightParamsSlot),
    };
    mGlobalBindingSet.reset(device->CreateResourceBindingSet(
        ResourceBindingSetDesc(globalBindings, 2, ShaderType::All)));
    if (!mGlobalBindingSet)
        return false;

    // G-Buffer binding set
    ResourceBindingDesc gbufferBindings[2] =
    {
        ResourceBindingDesc(ShaderResourceType::Texture, gbufferTex0Slot),
        ResourceBindingDesc(ShaderResourceType::Texture, gbufferTex1Slot),
        // ResourceBindingDesc(ShaderResourceType::Texture, gbufferTex2Slot),
        // ResourceBindingDesc(ShaderResourceType::Texture, gbufferTex3Slot),
    };
    mGBufferBindingSet.reset(device->CreateResourceBindingSet(
        ResourceBindingSetDesc(gbufferBindings, 2, ShaderType::Pixel)));
    if (!mGBufferBindingSet)
        return false;

    // shadow map binding set
    ResourceBindingDesc shadowMapBindings(ShaderResourceType::Texture, shadowMapSlot);
    mShadowMapBindingSet.reset(device->CreateResourceBindingSet(
        ResourceBindingSetDesc(&shadowMapBindings, 1, ShaderType::Pixel)));
    if (!mShadowMapBindingSet)
        return false;

    // create binding layout
    IResourceBindingSet* sets[] = { mGlobalBindingSet.get(),
                                    mGlobalBindingSet.get(),
                                    mShadowMapBindingSet.get() };
    mResBindingLayout.reset(device->CreateResourceBindingLayout(
        ResourceBindingLayoutDesc(sets, 3)));
    if (!mResBindingLayout)
        return false;

    return true;
}

void LightsRenderer::OnEnter(RenderContext* context)
{
    context->commandBuffer->BeginDebugGroup("Lights Renderer stage");
}

void LightsRenderer::OnLeave(RenderContext* context)
{
    // TODO: allow "NULL" in the array
    const int clearTexturesNum = 7;
    ITexture* nullTextures[clearTexturesNum];
    for (int i = 0; i < clearTexturesNum; ++i)
        nullTextures[i] = mRenderer->GetDefaultDiffuseTexture();

    // unbound all the textures
    // context->commandBuffer->SetTextures(nullTextures, clearTexturesNum, ShaderType::Pixel);
    // FIXME

    context->commandBuffer->EndDebugGroup();
}

void LightsRenderer::SetUp(RenderContext* context, IRenderTarget* target, GeometryBuffer *gbuffer,
                           const CameraRenderDesc* camera)
{
    context->commandBuffer->SetRenderTarget(target);

    int width, height;
    target->GetDimensions(width, height);
    context->commandBuffer->SetViewport(0.0f, static_cast<float>(width),
                                        0.0f, static_cast<float>(height),
                                        0.0f, 1.0f);

    // FIXME
    /*
    /// bind gbuffer to pixel shader
    ITexture* textures[] = { gbuffer->mTextures[0].get(),
        gbuffer->mTextures[1].get(),
        gbuffer->mTextures[2].get(),
        gbuffer->mTextures[3].get(),
        gbuffer->mDepthBuffer.get() };
    context->commandBuffer->SetTextures(textures, 5, ShaderType::Pixel);
    */

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
    context->commandBuffer->SetPipelineState(mAmbientLightPipelineState.get());

    AmbientLightCBuffer cbuffer;
    VectorStore(ambientLightColor, &cbuffer.ambientLight);
    VectorStore(backgroundColor, &cbuffer.backgroundColor);
    context->commandBuffer->WriteBuffer(mAmbientLightCBuffer.get(), 0, sizeof(AmbientLightCBuffer),
                                        &cbuffer);

    context->commandBuffer->SetShaderProgram(mAmbientLightShaderProgram.GetShaderProgram(nullptr));

    // FIXME
    // IBuffer* cbuffers[] = { mAmbientLightCBuffer.get() };
    // context->commandBuffer->SetConstantBuffers(cbuffers, 1, ShaderType::Pixel);

    context->commandBuffer->DrawIndexed(PrimitiveType::Triangles, 6);
}

void LightsRenderer::DrawOmniLight(RenderContext* context, const Vector& pos, float radius,
                                   const Vector& color, ShadowMap* shadowMap)
{
    // TODO: use instancing to draw lights

    context->commandBuffer->SetPipelineState(mLightVolumePipelineState.get());
    context->commandBuffer->BindResources(0, mOmniLightBindingInstance.get());

    OmniLightCBuffer cbuffer;
    cbuffer.position = pos;
    cbuffer.radius = VectorSplat(radius);
    cbuffer.color = color;
    cbuffer.shadowMapProps = Vector();

    int macros[] = { 0, 0 };
    if (shadowMap)
    {
        macros[mOmniLightUseShadowMap] = 1;
        cbuffer.shadowMapProps = Vector(1.0f / shadowMap->GetSize());

        // FIXME
        // ITexture* shadowMapTexture = shadowMap->mTexture.get();
        // context->commandBuffer->SetTextures(&shadowMapTexture, 1, ShaderType::Pixel, 6);
    }

    context->commandBuffer->SetShaderProgram(mOmniLightShaderProgram.GetShaderProgram(macros));

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
    // TODO: use instancing to draw lights

    int macros[] = { 0, 0 };

    if (lightMap != nullptr)
    {
        macros[mSpotLightUseLightMap] = 1;
        // FIXME
        // context->commandBuffer->SetTextures(&lightMap, 1, ShaderType::Pixel, 5);
    }

    if (shadowMap && shadowMap->mTexture)
    {
        macros[mSpotLightUseShadowMap] = 1;

        // FIXME
        // ITexture* shadowMapTexture = shadowMap->mTexture.get();
        // context->commandBuffer->SetTextures(&shadowMapTexture, 1, ShaderType::Pixel, 6);
    }

    context->commandBuffer->SetPipelineState(mLightVolumePipelineState.get());
    context->commandBuffer->SetShaderProgram(mSpotLightShaderProgram.GetShaderProgram(macros));
    context->commandBuffer->BindResources(0, mSpotLightBindingInstance.get());

    context->commandBuffer->WriteBuffer(mSpotLightCBuffer.get(), 0, sizeof(SpotLightProperties),
                                        &prop);

    context->commandBuffer->DrawIndexed(PrimitiveType::Triangles,
                                        2 * 6 * 3,  // 2 triangles per 6 sides
                                        -1,         // no instancing
                                        6 + 20 * 3, // ignore indices
                                        4 + 12);    // ignore vertices
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
