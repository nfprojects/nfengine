/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of Lights Renderer
 */

#pragma once

#include "PCH.hpp"
#include "LightsRenderer.hpp"

#include "nfCommon/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {

namespace {

struct NFE_ALIGN(32) LightsGlobalCBuffer
{
    Matrix4 cameraMatrix;
    Matrix4 viewMatrix;
    Matrix4 projMatrix;
    Vector4 viewportResInv;
    Vector4 screenScale;
};

struct AmbientLightCBuffer
{
    Float4 ambientLight;
    Float4 backgroundColor;
};

struct NFE_ALIGN(16) OmniLightCBuffer
{
    Vector4 position;
    Vector4 radius;
    Vector4 color;
    Vector4 shadowMapProps;
};

} // namespace

// renderer modules instance definition
std::unique_ptr<LightsRenderer> LightsRenderer::mPtr;


LightsRenderer::LightsRenderer()
{
    using namespace Resource;

    IDevice* device = mRenderer->GetDevice();

    mAmbientLightPipelineState.Load("AmbientLight");

    mOmniLightPipelineState.Load("OmniLight");
    mOmniLightUseShadowMap = mOmniLightPipelineState.GetMacroByName("USE_SHADOW_MAP");

    mSpotLightPipelineState.Load("SpotLight");
    mSpotLightUseLightMap = mSpotLightPipelineState.GetMacroByName("USE_LIGHT_MAP");
    mSpotLightUseShadowMap = mSpotLightPipelineState.GetMacroByName("USE_SHADOW_MAP");


    /// create vertex layout
    VertexLayoutElement vertexLayoutElements[] =
    {
        { ElementFormat::R32G32B32_Float, 0, 0, false, 0 }, // position
    };
    VertexLayoutDesc vertexLayoutDesc;
    vertexLayoutDesc.elements = vertexLayoutElements;
    vertexLayoutDesc.numElements = 1;
    vertexLayoutDesc.debugName = "LightsRenderer::mVertexLayout";
    mVertexLayout= device->CreateVertexLayout(vertexLayoutDesc);

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
    bufferDesc.mode = BufferMode::Static;
    bufferDesc.size = sizeof(vertices);
    bufferDesc.type = BufferType::Vertex;
    bufferDesc.initialData = vertices;
    bufferDesc.debugName = "LightsRenderer::mVertexBuffer";
    mVertexBuffer= device->CreateBuffer(bufferDesc);

    bufferDesc.mode = BufferMode::Static;
    bufferDesc.size = sizeof(indices);
    bufferDesc.type = BufferType::Index;
    bufferDesc.initialData = indices;
    bufferDesc.debugName = "LightsRenderer::mIndexBuffer";
    mIndexBuffer= device->CreateBuffer(bufferDesc);

    // constant buffers
    {
        bufferDesc.mode = BufferMode::Volatile;
        bufferDesc.type = BufferType::Constant;
        bufferDesc.initialData = nullptr;

        bufferDesc.size = sizeof(LightsGlobalCBuffer);
        bufferDesc.debugName = "LightsRenderer::mGlobalCBuffer";
        mGlobalCBuffer= device->CreateBuffer(bufferDesc);

        bufferDesc.size = sizeof(AmbientLightCBuffer);
        bufferDesc.debugName = "LightsRenderer::mAmbientLightCBuffer";
        mAmbientLightCBuffer= device->CreateBuffer(bufferDesc);

        bufferDesc.size = sizeof(OmniLightCBuffer);
        bufferDesc.debugName = "LightsRenderer::mOmniLightCBuffer";
        mOmniLightCBuffer= device->CreateBuffer(bufferDesc);

        bufferDesc.size = sizeof(SpotLightProperties);
        bufferDesc.debugName = "LightsRenderer::mSpotLightCBuffer";
        mSpotLightCBuffer= device->CreateBuffer(bufferDesc);
    }

    // shadow map sampler
    SamplerDesc samplerDesc;
    samplerDesc.magFilter = TextureMagFilter::Linear;
    samplerDesc.minFilter = TextureMinFilter::NearestMipmapLinear;
    samplerDesc.debugName = "LightsRenderer::mShadowMapSampler";
    samplerDesc.compare = true;
    samplerDesc.wrapModeU = samplerDesc.wrapModeV = TextureWrapMode::Clamp;
    samplerDesc.compareFunc = CompareFunc::LessEqual;
    mShadowMapSampler= device->CreateSampler(samplerDesc);

    CreateResourceBindingLayouts();

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.resBindingLayout = mResBindingLayout;
    pipelineStateDesc.vertexLayout = mVertexLayout;
    pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
    pipelineStateDesc.rtFormats[0] = ElementFormat::R16G16B16A16_Float;
    pipelineStateDesc.numRenderTargets = 1;

    pipelineStateDesc.debugName = "LightsRenderer::mAmbientLightPipelineState";
    mAmbientLightPipelineState.Build(pipelineStateDesc);

    // blend state that enables additive alpha-blending
    pipelineStateDesc.blendState.rtDescs[0].enable = true;
    // depth state for light volumes rendering
    pipelineStateDesc.depthState.depthCompareFunc = CompareFunc::Greater;
    pipelineStateDesc.depthState.depthTestEnable = true;
    pipelineStateDesc.depthState.depthWriteEnable = false;
    // rasterizer state for light volumes rendering
    pipelineStateDesc.raterizerState.cullMode = CullMode::CCW;

    pipelineStateDesc.debugName = "LightsRenderer::mOmniLightPipelineState";
    mOmniLightPipelineState.Build(pipelineStateDesc);

    pipelineStateDesc.debugName = "LightsRenderer::mSpotLightPipelineState";
    mSpotLightPipelineState.Build(pipelineStateDesc);
}

bool LightsRenderer::CreateResourceBindingLayouts()
{
    IDevice* device = mRenderer->GetDevice();

    int globalCBufferSlot = mOmniLightPipelineState.GetResourceSlotByName("Global");
    int lightParamsSlot = mOmniLightPipelineState.GetResourceSlotByName("OmniLightProps");
    // TODO: verify other light types
    if (globalCBufferSlot < 0 || lightParamsSlot < 0)
    {
        NFE_LOG_ERROR("Invalid cbuffer slot");
        return false;
    }

    int depthTexSlot = mOmniLightPipelineState.GetResourceSlotByName("gDepthTex");
    int gbufferTex0Slot = mOmniLightPipelineState.GetResourceSlotByName("gGBufferTex0");
    int gbufferTex1Slot = mOmniLightPipelineState.GetResourceSlotByName("gGBufferTex1");
    int gbufferTex2Slot = mAmbientLightPipelineState.GetResourceSlotByName("gGBufferTex2"); // TODO
    // int gbufferTex3Slot = mOmniLightPipelineState.GetResourceSlotByName("gGBufferTex3");
    if (depthTexSlot < 0 || gbufferTex0Slot < 0 || gbufferTex1Slot < 0
        /* || gbufferTex2Slot < 0 || gbufferTex3Slot < 0 */ )
    {
        NFE_LOG_ERROR("Invalid gbuffer slot");
        return false;
    }

    int shadowMapSlot = mOmniLightPipelineState.GetResourceSlotByName("gShadowMap");
    if (shadowMapSlot < 0)
    {
        NFE_LOG_ERROR("Invalid shadowMapSlot slot");
        return false;
    }

    int lightMapSlot = mSpotLightPipelineState.GetResourceSlotByName("gLightMap");
    if (lightMapSlot < 0)
    {
        NFE_LOG_ERROR("Invalid lightMapSlot slot");
        return false;
    }

    // G-Buffer binding set
    ResourceBindingDesc gbufferBindings[4] =
    {
        ResourceBindingDesc(ShaderResourceType::Texture, depthTexSlot),
        ResourceBindingDesc(ShaderResourceType::Texture, gbufferTex0Slot),
        ResourceBindingDesc(ShaderResourceType::Texture, gbufferTex1Slot),
        ResourceBindingDesc(ShaderResourceType::Texture, gbufferTex2Slot),
        // ResourceBindingDesc(ShaderResourceType::Texture, gbufferTex3Slot),
    };
    mGBufferBindingSet = device->CreateResourceBindingSet(
        ResourceBindingSetDesc(gbufferBindings, 4, ShaderType::Pixel));
    if (!mGBufferBindingSet)
        return false;

    // shadow map binding set
    ResourceBindingDesc shadowMapBindings(ShaderResourceType::Texture, shadowMapSlot,
                                          mShadowMapSampler);
    mShadowMapBindingSet = device->CreateResourceBindingSet(
        ResourceBindingSetDesc(&shadowMapBindings, 1, ShaderType::Pixel));
    if (!mShadowMapBindingSet)
        return false;

    // light map binding set
    ResourceBindingDesc lightMapBindings(ShaderResourceType::Texture, lightMapSlot,
                                         mRenderer->GetDefaultSampler());
    mLightMapBindingSet = device->CreateResourceBindingSet(
        ResourceBindingSetDesc(&lightMapBindings, 1, ShaderType::Pixel));
    if (!mLightMapBindingSet)
        return false;

    VolatileCBufferBinding cbufferBindingsDesc[2] =
    {
        VolatileCBufferBinding(ShaderType::All, ShaderResourceType::CBuffer, globalCBufferSlot, sizeof(LightsGlobalCBuffer)),
        VolatileCBufferBinding(ShaderType::All, ShaderResourceType::CBuffer, lightParamsSlot, sizeof(OmniLightCBuffer)),
    };

    // create binding layout
    ResourceBindingSetPtr sets[] =
    {
        mGBufferBindingSet,
        mShadowMapBindingSet,
        mLightMapBindingSet
    };
    mResBindingLayout = device->CreateResourceBindingLayout( ResourceBindingLayoutDesc(sets, 3, cbufferBindingsDesc, 2));
    if (!mResBindingLayout)
        return false;

    return true;
}

void LightsRenderer::OnEnter(LightsRendererContext* context)
{
    context->commandRecorder->BeginDebugGroup("Lights Renderer stage");
    context->commandRecorder->SetResourceBindingLayout(mResBindingLayout);
}

void LightsRenderer::OnLeave(LightsRendererContext* context)
{
    context->commandRecorder->EndDebugGroup();
}

void LightsRenderer::SetUp(LightsRendererContext* context, const RenderTargetPtr& target, GeometryBuffer *gbuffer,
                           const CameraRenderDesc* camera)
{
    context->commandRecorder->SetRenderTarget(target);

    int width, height;
    target->GetDimensions(width, height);
    context->commandRecorder->SetViewport(0.0f, static_cast<float>(width),
                                        0.0f, static_cast<float>(height),
                                        0.0f, 1.0f);
    context->commandRecorder->SetScissors(0, 0, width, height);
    context->commandRecorder->BindResources(0, gbuffer->mBindingInstance);
    context->commandRecorder->BindVolatileCBuffer(0, mGlobalCBuffer);

    BufferPtr buffers[] = { mVertexBuffer };
    uint32 strides[] = { sizeof(Float3) };
    uint32 offsets[] = { 0 };
    context->commandRecorder->SetVertexBuffers(1, buffers, strides, offsets);
    context->commandRecorder->SetIndexBuffer(mIndexBuffer, IndexBufferFormat::Uint16);

    LightsGlobalCBuffer cbuffer;
    cbuffer.cameraMatrix = camera->matrix;
    cbuffer.projMatrix = camera->projMatrix;
    cbuffer.viewMatrix = camera->viewMatrix;
    cbuffer.viewportResInv = Vector4(1.0f / gbuffer->GetWidth(), 1.0f / gbuffer->GetHeight());
    cbuffer.screenScale = camera->screenScale;
    context->commandRecorder->WriteBuffer(mGlobalCBuffer, 0, sizeof(LightsGlobalCBuffer),
                                        &cbuffer);
}

void LightsRenderer::DrawAmbientLight(LightsRendererContext* context, const Vector4& ambientLightColor,
                                      const Vector4& backgroundColor)
{
    context->commandRecorder->SetPipelineState(mAmbientLightPipelineState.GetPipelineState(nullptr));
    context->commandRecorder->BindVolatileCBuffer(0, mGlobalCBuffer);
    context->commandRecorder->BindVolatileCBuffer(1, mAmbientLightCBuffer);

    AmbientLightCBuffer cbuffer;
    cbuffer.ambientLight = ambientLightColor.ToFloat4();
    cbuffer.backgroundColor = backgroundColor.ToFloat4();
    context->commandRecorder->WriteBuffer(mAmbientLightCBuffer, 0, sizeof(AmbientLightCBuffer),
                                        &cbuffer);

    context->commandRecorder->DrawIndexed(6);
}

void LightsRenderer::DrawOmniLight(LightsRendererContext* context, const Vector4& pos, float radius,
                                   const Vector4& color, ShadowMap* shadowMap)
{
    // TODO: use instancing to draw lights

    OmniLightCBuffer cbuffer;
    cbuffer.position = pos;
    cbuffer.radius = Vector4(radius);
    cbuffer.color = color;
    cbuffer.shadowMapProps = Vector4::Zero();

    int macros[] = { 0, 0 };
    if (shadowMap)
    {
        macros[mOmniLightUseShadowMap] = 1;
        cbuffer.shadowMapProps = Vector4(1.0f / shadowMap->GetSize());

        context->commandRecorder->BindResources(1, shadowMap->mBindingInstance);
    }

    context->commandRecorder->SetPipelineState(mOmniLightPipelineState.GetPipelineState(macros));
    context->commandRecorder->BindVolatileCBuffer(0, mGlobalCBuffer);
    context->commandRecorder->BindVolatileCBuffer(1, mOmniLightCBuffer);

    context->commandRecorder->WriteBuffer(mOmniLightCBuffer, 0, sizeof(OmniLightCBuffer),
                                        &cbuffer);

    context->commandRecorder->DrawIndexed(20 * 3, // 20 triangles
                                        1,      // no instancing
                                        6,      // ignore first 6 indices
                                        4);     // ignore first 4 vertices
}

void LightsRenderer::DrawSpotLight(LightsRendererContext* context, const SpotLightProperties& prop,
                                   ShadowMap* shadowMap, const ResourceBindingInstancePtr& lightMap)
{
    // TODO: use instancing to draw lights

    int macros[] = { 0, 0 };

    // FIXME
    if (lightMap != nullptr)
    {
        macros[mSpotLightUseLightMap] = 1;
        context->commandRecorder->BindResources(2, lightMap);
    }

    if (shadowMap && shadowMap->mTexture)
    {
        macros[mSpotLightUseShadowMap] = 1;
        context->commandRecorder->BindResources(1, shadowMap->mBindingInstance);
    }

    context->commandRecorder->SetPipelineState(mSpotLightPipelineState.GetPipelineState(macros));
    context->commandRecorder->BindVolatileCBuffer(0, mGlobalCBuffer);
    context->commandRecorder->BindVolatileCBuffer(1, mSpotLightCBuffer);

    context->commandRecorder->WriteBuffer(mSpotLightCBuffer, 0, sizeof(SpotLightProperties),
                                        &prop);

    context->commandRecorder->DrawIndexed(2 * 6 * 3,  // 2 triangles per 6 sides
                                        1,          // no instancing
                                        6 + 20 * 3, // ignore indices
                                        4 + 12);    // ignore vertices
}

void LightsRenderer::DrawDirLight(LightsRendererContext* context, const DirLightProperties& prop,
                                  ShadowMap* shadowMap)
{
}

void LightsRenderer::DrawFog(LightsRendererContext* context)
{
}

} // namespace Renderer
} // namespace NFE
