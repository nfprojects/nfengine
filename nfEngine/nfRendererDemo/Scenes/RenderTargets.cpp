/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  RenderTargets scene definition
 */

#include "../PCH.hpp"

#include "RenderTargets.hpp"
#include "../Common.hpp"

#include "../nfCommon/Math/Matrix.hpp"

#include <vector>
#include <functional>

using namespace NFE::Math;
using namespace NFE::Common;
using namespace NFE::Renderer;

namespace {

// TODO: this should be obtained in the runtime
const int MULTISAMPLE_SAMPLES = 8;

struct VertexCBuffer
{
    Matrix viewMatrix;
};

} // namespace

RenderTargetsScene::RenderTargetsScene()
    : Scene("RenderTarget")
{
    RegisterSubScene(std::bind(&RenderTargetsScene::CreateSubSceneNoDepthBuffer, this),
                     "Single target without depth buffer");
    RegisterSubScene(std::bind(&RenderTargetsScene::CreateSubSceneDepthBuffer, this),
                     "Single target with depth buffer");
    RegisterSubScene(std::bind(&RenderTargetsScene::CreateSubSceneMRT, this),
                     "Two targets with depth buffer");
    RegisterSubScene(std::bind(&RenderTargetsScene::CreateSubSceneMRTandMSAA, this),
                     "Two targets with depth buffer (with MSAA)");
}

RenderTargetsScene::~RenderTargetsScene()
{
    Release();
}

bool RenderTargetsScene::CreateBasicResources()
{
    // create rendertarget that will render to the window's backbuffer
    RenderTargetElement rtTarget;
    rtTarget.texture = mWindowBackbuffer.get();
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;
    rtDesc.depthBuffer = nullptr;
    rtDesc.debugName = "RenderTargetsScene::mWindowRenderTarget";
    mWindowRenderTarget.reset(mRendererDevice->CreateRenderTarget(rtDesc));
    if (!mWindowRenderTarget)
        return false;

    DepthStateDesc depthStateDesc;
    depthStateDesc.depthCompareFunc = CompareFunc::Less;
    depthStateDesc.depthWriteEnable = true;
    depthStateDesc.depthTestEnable = true;
    depthStateDesc.stencilEnable = false;
    mDepthState.reset(mRendererDevice->CreateDepthState(depthStateDesc));
    if (!mDepthState)
        return false;

    SamplerDesc samplerDesc;
    mSampler.reset(mRendererDevice->CreateSampler(samplerDesc));
    if (!mSampler)
        return false;

    RasterizerStateDesc rasterizerStateDesc;
    rasterizerStateDesc.cullMode = CullMode::Disabled;
    mRasterizerState.reset(mRendererDevice->CreateRasterizerState(rasterizerStateDesc));
    if (!mRasterizerState)
        return false;

    // create vertex buffers
    float vbData[] =
    {
        // Vertex structure: pos.xyz, texCoord.uv, color.rgba

        // cube
        -1.0f, -1.0f, -1.0f,  0.0f, 0.0f,  0.1f, 0.6f, 0.9f, 1.0f,
        -1.0f, -1.0f,  1.0f,  0.0f, 0.0f,  1.0f, 0.3f, 0.7f, 1.0f,
        -1.0f,  1.0f, -1.0f,  0.0f, 0.0f,  0.4f, 0.0f, 0.9f, 1.0f,
        -1.0f,  1.0f,  1.0f,  0.0f, 0.0f,  0.3f, 1.6f, 0.7f, 1.0f,
         1.0f, -1.0f, -1.0f,  0.0f, 0.0f,  0.1f, 0.7f, 0.2f, 1.0f,
         1.0f, -1.0f,  1.0f,  0.0f, 0.0f,  0.7f, 0.1f, 0.1f, 1.0f,
         1.0f,  1.0f, -1.0f,  0.0f, 0.0f,  0.8f, 0.3f, 0.5f, 1.0f,
         1.0f,  1.0f,  1.0f,  0.0f, 0.0f,  0.5f, 0.4f, 1.9f, 1.0f,

        // quad
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,   1.0f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,
         1.0f,  1.0f, 0.0f,   1.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f,
    };

    const uint16 ibData[] =
    {
        // cube
        0, 1, 3,  0, 3, 2,
        1, 5, 7,  1, 7, 3,
        5, 6, 4,  5, 7, 6,
        4, 2, 0,  4, 6, 2,
        2, 3, 7,  2, 7, 6,
        4, 1, 0,  4, 5, 1,

        // quad
        8, 9, 10,  8, 10, 11,
    };

    BufferDesc bufferDesc;
    bufferDesc.type = BufferType::Vertex;
    bufferDesc.access = BufferAccess::GPU_ReadOnly;
    bufferDesc.size = sizeof(vbData);
    bufferDesc.initialData = vbData;
    mVertexBuffer.reset(mRendererDevice->CreateBuffer(bufferDesc));
    if (!mVertexBuffer)
        return false;

    bufferDesc.type = BufferType::Index;
    bufferDesc.access = BufferAccess::GPU_ReadOnly;
    bufferDesc.size = sizeof(ibData);
    bufferDesc.initialData = ibData;
    mIndexBuffer.reset(mRendererDevice->CreateBuffer(bufferDesc));
    if (!mIndexBuffer)
        return false;

    VertexLayoutElement vertexLayoutElements[] =
    {
        { ElementFormat::Float_32, 3,  0, 0, false, 0 }, // position
        { ElementFormat::Float_32, 2, 12, 0, false, 0 }, // tex-coords
        { ElementFormat::Float_32, 4, 20, 0, false, 0 }, // color
    };

    VertexLayoutDesc vertexLayoutDesc;
    vertexLayoutDesc.elements = vertexLayoutElements;
    vertexLayoutDesc.numElements = 3;
    vertexLayoutDesc.vertexShader = mVertexShader.get();
    mVertexLayout.reset(mRendererDevice->CreateVertexLayout(vertexLayoutDesc));
    if (!mVertexLayout)
        return false;

    bufferDesc.type = BufferType::Constant;
    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = sizeof(VertexCBuffer);
    bufferDesc.initialData = nullptr;
    mConstantBuffer.reset(mRendererDevice->CreateBuffer(bufferDesc));
    if (!mConstantBuffer)
        return false;

    return true;
}

bool RenderTargetsScene::CreateRenderTarget(bool withDepthBuffer, bool multipleRT, bool withMSAA)
{
    TextureDesc texDesc;
    texDesc.type = TextureType::Texture2D;
    texDesc.access = BufferAccess::GPU_ReadWrite;
    texDesc.width = WINDOW_WIDTH / 2;
    texDesc.height = WINDOW_HEIGHT / 2;
    texDesc.mipmaps = 1;
    texDesc.samplesNum = withMSAA ? MULTISAMPLE_SAMPLES : 1;

    // render target texture
    texDesc.format = ElementFormat::Uint_8_norm;
    texDesc.texelSize = 4;
    texDesc.binding = NFE_RENDERER_TEXTURE_BIND_RENDERTARGET | NFE_RENDERER_TEXTURE_BIND_SHADER;
    texDesc.debugName = "RenderTargetsScene::mRenderTargetTexture[0]";
    mRenderTargetTextures[0].reset(mRendererDevice->CreateTexture(texDesc));
    if (!mRenderTargetTextures[0])
        return false;

    if (multipleRT)
    {
        texDesc.debugName = "RenderTargetsScene::mRenderTargetTexture[1]";
        mRenderTargetTextures[1].reset(mRendererDevice->CreateTexture(texDesc));
        if (!mRenderTargetTextures[1])
            return false;
    }

    // render target's depth buffer
    if (withDepthBuffer)
    {
        texDesc.binding = NFE_RENDERER_TEXTURE_BIND_DEPTH | NFE_RENDERER_TEXTURE_BIND_SHADER;
        texDesc.depthBufferFormat = DepthBufferFormat::Depth16;
        texDesc.debugName = "RenderTargetsScene::mDepthBuffer";
        mDepthBuffer.reset(mRendererDevice->CreateTexture(texDesc));
        if (!mDepthBuffer)
            return false;
    }

    RenderTargetElement rtTargets[2];
    rtTargets[0].texture = mRenderTargetTextures[0].get();
    rtTargets[1].texture = mRenderTargetTextures[1].get();
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = multipleRT ? 2 : 1;
    rtDesc.targets = rtTargets;
    rtDesc.depthBuffer = withDepthBuffer ? mDepthBuffer.get() : nullptr;
    rtDesc.debugName = "RenderTargetsScene::mRenderTarget";
    mRenderTarget.reset(mRendererDevice->CreateRenderTarget(rtDesc));
    if (!mRenderTarget)
        return false;

    return true;
}

bool RenderTargetsScene::CreateShaders(bool multipleRT, bool withMSAA)
{
    // Common Vertex Shader

    ShaderMacro vsMacro[] = { { "USE_CBUFFER", "1" } };
    std::string vsPath = gShaderPathPrefix + "TestVS" + gShaderPathExt;
    mVertexShader.reset(CompileShader(vsPath.c_str(), ShaderType::Vertex, vsMacro, 1));
    if (!mVertexShader)
        return false;

    // Pixel Shader for 3D cube rendering

    ShaderMacro psMacroRT[] = { { "TARGETS", multipleRT ? "2" : "1" } };
    std::string psPath = gShaderPathPrefix + "RenderTargetPS" + gShaderPathExt;
    mRTPixelShader.reset(CompileShader(psPath.c_str(), ShaderType::Pixel, psMacroRT, 1));
    if (!mRTPixelShader)
        return false;
    ShaderProgramDesc shaderProgramDesc;
    shaderProgramDesc.vertexShader = mVertexShader.get();
    shaderProgramDesc.pixelShader = mRTPixelShader.get();
    mRTShaderProgram.reset(mRendererDevice->CreateShaderProgram(shaderProgramDesc));
    if (!mRTShaderProgram)
        return false;

    char samplesNumStr[8];
    snprintf(samplesNumStr, 8, "%i", withMSAA ? MULTISAMPLE_SAMPLES : 1);

    // Pixel Shader for primary render target preview

    ShaderMacro psMacrosNormal[] = { { "MODE", "0" }, { "SAMPLES_NUM", samplesNumStr } };
    psPath = gShaderPathPrefix + "PostProcessPS" + gShaderPathExt;
    mPrimaryTargetPixelShader.reset(CompileShader(psPath.c_str(), ShaderType::Pixel,
                                                  psMacrosNormal, 2));
    if (!mPrimaryTargetPixelShader)
        return false;
    shaderProgramDesc.vertexShader = mVertexShader.get();
    shaderProgramDesc.pixelShader = mPrimaryTargetPixelShader.get();
    mPrimaryTargetShaderProgram.reset(mRendererDevice->CreateShaderProgram(shaderProgramDesc));
    if (!mPrimaryTargetShaderProgram)
        return false;

    // Pixel Shader for depthbuffer preview

    ShaderMacro psMacroDepth[] = { { "MODE", "1" }, { "SAMPLES_NUM", samplesNumStr } };
    psPath = gShaderPathPrefix + "PostProcessPS" + gShaderPathExt;
    mDepthPixelShader.reset(CompileShader(psPath.c_str(), ShaderType::Pixel, psMacroDepth, 2));
    if (!mDepthPixelShader)
        return false;
    shaderProgramDesc.vertexShader = mVertexShader.get();
    shaderProgramDesc.pixelShader = mDepthPixelShader.get();
    mDepthShaderProgram.reset(mRendererDevice->CreateShaderProgram(shaderProgramDesc));
    if (!mDepthShaderProgram)
        return false;

    // Pixel Shader for secondary render target preview

    ShaderMacro psMacroSecondary[] = { { "MODE", "2" }, { "SAMPLES_NUM", samplesNumStr } };
    psPath = gShaderPathPrefix + "PostProcessPS" + gShaderPathExt;
    mSecondTargetPixelShader.reset(CompileShader(psPath.c_str(), ShaderType::Pixel,
                                                 psMacroSecondary, 2));
    if (!mSecondTargetPixelShader)
        return false;
    shaderProgramDesc.vertexShader = mVertexShader.get();
    shaderProgramDesc.pixelShader = mSecondTargetPixelShader.get();
    mSecondTargetShaderProgram.reset(mRendererDevice->CreateShaderProgram(shaderProgramDesc));
    if (!mSecondTargetShaderProgram)
        return false;

    return true;
}

bool RenderTargetsScene::CreateSubSceneNoDepthBuffer()
{
    if (!CreateShaders())
        return false;
    if (!CreateBasicResources())
        return false;
    return CreateBasicResources();
}

bool RenderTargetsScene::CreateSubSceneDepthBuffer()
{
    if (!CreateShaders())
        return false;
    if (!CreateBasicResources())
        return false;
    return CreateRenderTarget(true);
}

bool RenderTargetsScene::CreateSubSceneMRT()
{
    if (!CreateShaders(true))
        return false;
    if (!CreateBasicResources())
        return false;
    return CreateRenderTarget(true, true);
}

bool RenderTargetsScene::CreateSubSceneMRTandMSAA()
{
    if (!CreateShaders(true, true))
        return false;
    if (!CreateBasicResources())
        return false;
    return CreateRenderTarget(true, true, true);
}

bool RenderTargetsScene::OnInit(void* winHandle)
{
    // create backbuffer connected with the window
    BackbufferDesc bbDesc;
    bbDesc.width = WINDOW_WIDTH;
    bbDesc.height = WINDOW_HEIGHT;
    bbDesc.windowHandle = winHandle;
    bbDesc.vSync = true;
    bbDesc.debugName = "RenderTargetsScene::mWindowBackbuffer";
    mWindowBackbuffer.reset(mRendererDevice->CreateBackbuffer(bbDesc));
    if (!mWindowBackbuffer)
        return false;

    return true;
}

bool RenderTargetsScene::OnSwitchSubscene()
{
    mAngle = 0.0f;

    mCommandBuffer->Reset();

    int stride = 9 * sizeof(float);
    int offset = 0;
    IBuffer* vb = mVertexBuffer.get();
    mCommandBuffer->SetVertexBuffers(1, &vb, &stride, &offset);
    mCommandBuffer->SetIndexBuffer(mIndexBuffer.get(), IndexBufferFormat::Uint16);

    mCommandBuffer->SetVertexLayout(mVertexLayout.get());
    mCommandBuffer->SetRasterizerState(mRasterizerState.get());

    ISampler* samplers[] = { mSampler.get() };
    mCommandBuffer->SetSamplers(samplers, 1, ShaderType::Pixel, 0);

    return true;
}

void RenderTargetsScene::Draw(float dt)
{
    mAngle += 1.0f * dt;
    if (mAngle > NFE_MATH_2PI)
        mAngle -= NFE_MATH_2PI;

    Matrix modelMatrix = MatrixRotationNormal(Vector(0.0f, 1.0f, 0.0f), mAngle);
    Matrix viewMatrix = MatrixLookTo(Vector(3.0f, 0.0f, 0.0f), Vector(-1.0f, 0.0f, 0.0f),
                                     Vector(0.0f, 1.0f, 0.0f));
    Matrix projMatrix = MatrixPerspective((float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
                                          70.0f * NFE_MATH_PI / 180.0f, 5.0f, 1.0f);

    IBuffer* cb = mConstantBuffer.get();
    mCommandBuffer->SetConstantBuffers(&cb, 1, ShaderType::Vertex);

    // render cube to a texture
    {
        mCommandBuffer->SetViewport(0.0f, (float)(WINDOW_WIDTH / 2),
                                    0.0f, (float)(WINDOW_HEIGHT / 2), 0.0f, 1.0f);

        VertexCBuffer cbuffer;
        cbuffer.viewMatrix = modelMatrix * viewMatrix * projMatrix;
        mCommandBuffer->WriteBuffer(cb, 0, sizeof(VertexCBuffer), &cbuffer);

        mCommandBuffer->SetRenderTarget(mRenderTarget.get());
        mCommandBuffer->SetShaderProgram(mRTShaderProgram.get());

        float color[] = { 0.2f, 0.3f, 0.4f, 1.0f };
        mCommandBuffer->Clear(NFE_CLEAR_FLAG_DEPTH | NFE_CLEAR_FLAG_TARGET, color, 1.0f);
        mCommandBuffer->DrawIndexed(PrimitiveType::Triangles, 2 * 6 * 3);
    }

    // begin rendering to the window
    mCommandBuffer->SetRenderTarget(mWindowRenderTarget.get());
    mCommandBuffer->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);

    float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    mCommandBuffer->Clear(NFE_CLEAR_FLAG_TARGET, color, 1.0f);

    // show primary render target texture with simple effect (blur)
    {
        VertexCBuffer cbuffer;
        cbuffer.viewMatrix = MatrixScaling(Vector(0.5f, 0.5f, 0.5f, 1.0f)) *
                             MatrixTranslation3(Vector(-0.5f, 0.5f, 0.5f));
        mCommandBuffer->WriteBuffer(cb, 0, sizeof(VertexCBuffer), &cbuffer);

        ITexture* textures[] = { mRenderTargetTextures[0].get() };
        mCommandBuffer->SetTextures(textures, 1, ShaderType::Pixel, 0);
        mCommandBuffer->SetShaderProgram(mPrimaryTargetShaderProgram.get());
        mCommandBuffer->DrawIndexed(PrimitiveType::Triangles,
                                    2 * 3,      // 2 triangles
                                    -1,         // no instancing
                                    2 * 6 * 3); // omit cube vertices
    }

    // show depth-buffer
    if (GetCurrentSubSceneNumber() > 0)
    {
        VertexCBuffer cbuffer;
        cbuffer.viewMatrix = MatrixScaling(Vector(0.5f, 0.5f, 0.5f, 1.0f)) *
            MatrixTranslation3(Vector(0.5f, 0.5f, 0.5f));
        mCommandBuffer->WriteBuffer(cb, 0, sizeof(VertexCBuffer), &cbuffer);

        ITexture* textures[] = { mDepthBuffer.get() };
        mCommandBuffer->SetTextures(textures, 1, ShaderType::Pixel, 0);
        mCommandBuffer->SetShaderProgram(mDepthShaderProgram.get());
        mCommandBuffer->DrawIndexed(PrimitiveType::Triangles,
                                    2 * 3,      // 2 triangles
                                    -1,         // no instancing
                                    2 * 6 * 3); // omit cube vertices
    }

    // show secondary render target texture
    if (GetCurrentSubSceneNumber() > 1)
    {
        VertexCBuffer cbuffer;
        cbuffer.viewMatrix = MatrixScaling(Vector(0.5f, 0.5f, 0.5f, 1.0f)) *
            MatrixTranslation3(Vector(-0.5f, -0.5f, 0.5f));
        mCommandBuffer->WriteBuffer(cb, 0, sizeof(VertexCBuffer), &cbuffer);

        ITexture* textures[] = { mRenderTargetTextures[1].get() };
        mCommandBuffer->SetTextures(textures, 1, ShaderType::Pixel, 0);
        mCommandBuffer->SetShaderProgram(mSecondTargetShaderProgram.get());
        mCommandBuffer->DrawIndexed(PrimitiveType::Triangles,
                                    2 * 3,      // 2 triangles
                                    -1,         // no instancing
                                    2 * 6 * 3); // omit cube vertices
    }

    ITexture* nullTextures[] = { nullptr };
    mCommandBuffer->SetTextures(nullTextures, 1, ShaderType::Pixel, 0);

    mWindowBackbuffer->Present();
}

void RenderTargetsScene::ReleaseSubsceneResources()
{
    // clear resources
    mWindowRenderTarget.reset();
    mRenderTargetTextures[0].reset();
    mRenderTargetTextures[1].reset();
    mDepthBuffer.reset();
    mRenderTarget.reset();
    mVertexShader.reset();
    mRTPixelShader.reset();
    mPrimaryTargetPixelShader.reset();
    mDepthPixelShader.reset();
    mSecondTargetPixelShader.reset();
    mRTShaderProgram.reset();
    mPrimaryTargetShaderProgram.reset();
    mDepthShaderProgram.reset();
    mSecondTargetShaderProgram.reset();
    mConstantBuffer.reset();
    mVertexBuffer.reset();
    mIndexBuffer.reset();
    mVertexLayout.reset();
    mSampler.reset();
    mDepthState.reset();
    mRasterizerState.reset();
}

void RenderTargetsScene::Release()
{
    ReleaseSubsceneResources();
    mWindowBackbuffer.reset();
    mCommandBuffer = nullptr;
    mRendererDevice = nullptr;
}
