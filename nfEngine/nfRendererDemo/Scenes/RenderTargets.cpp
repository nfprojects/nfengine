/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  RenderTargets scene definition
 */

#include "../PCH.hpp"

#include "RenderTargets.hpp"
#include "../Common.hpp"

#include "nfCommon/Math/Matrix.hpp"

#include <vector>
#include <functional>


using namespace NFE;
using namespace NFE::Math;
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

bool RenderTargetsScene::CreateBasicResources(bool multipleRT, bool withDepthBuffer, bool withMSAA)
{
    // create rendertarget that will render to the window's backbuffer
    RenderTargetElement rtTarget;
    rtTarget.texture = mWindowBackbuffer;
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;
    rtDesc.depthBuffer = nullptr;
    rtDesc.debugName = "RenderTargetsScene::mWindowRenderTarget";
    mWindowRenderTarget = mRendererDevice->CreateRenderTarget(rtDesc);
    if (!mWindowRenderTarget)
        return false;

    VertexLayoutElement vertexLayoutElements[] =
    {
        { ElementFormat::R32G32B32_Float,       0, 0, false, 0 }, // position
        { ElementFormat::R32G32_Float,          12, 0, false, 0 }, // tex-coords
        { ElementFormat::R32G32B32A32_Float,    20, 0, false, 0 }, // color
    };

    VertexLayoutDesc vertexLayoutDesc;
    vertexLayoutDesc.elements = vertexLayoutElements;
    vertexLayoutDesc.numElements = 3;
    mVertexLayout = mRendererDevice->CreateVertexLayout(vertexLayoutDesc);
    if (!mVertexLayout)
        return false;

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.rtFormats[0] = mBackbufferFormat;
    pipelineStateDesc.vertexShader = mVertexShader;
    pipelineStateDesc.depthFormat = DepthBufferFormat::Unknown;
    pipelineStateDesc.resBindingLayout = mResBindingLayout;
    pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
    pipelineStateDesc.vertexLayout = mVertexLayout;
    pipelineStateDesc.depthState.depthCompareFunc = CompareFunc::Less;
    pipelineStateDesc.depthState.depthWriteEnable = false;
    pipelineStateDesc.depthState.depthTestEnable = false;
    pipelineStateDesc.depthState.stencilEnable = false;
    pipelineStateDesc.raterizerState.cullMode = CullMode::Disabled;

    pipelineStateDesc.pixelShader = mPrimaryTargetPixelShader;
    mPrimaryTargetPipelineState = mRendererDevice->CreatePipelineState(pipelineStateDesc);
    if (!mPrimaryTargetPipelineState)
        return false;

    pipelineStateDesc.pixelShader = mDepthPixelShader;
    mDepthPipelineState = mRendererDevice->CreatePipelineState(pipelineStateDesc);
    if (!mDepthPipelineState)
        return false;

    pipelineStateDesc.pixelShader = mSecondTargetPixelShader;
    mSecondTargetPipelineState = mRendererDevice->CreatePipelineState(pipelineStateDesc);
    if (!mSecondTargetPipelineState)
        return false;

    pipelineStateDesc.rtFormats[0] = ElementFormat::R8G8B8A8_U_Norm;
    pipelineStateDesc.rtFormats[1] = ElementFormat::R8G8B8A8_U_Norm;
    pipelineStateDesc.samplesNum = withMSAA ? MULTISAMPLE_SAMPLES : 1;
    pipelineStateDesc.pixelShader = mRTPixelShader;
    pipelineStateDesc.depthFormat = withDepthBuffer ? DepthBufferFormat::Depth16 : DepthBufferFormat::Unknown;
    pipelineStateDesc.numRenderTargets = multipleRT ? 2 : 1;
    pipelineStateDesc.depthState.depthWriteEnable = withDepthBuffer;
    pipelineStateDesc.depthState.depthTestEnable = withDepthBuffer;
    mPipelineStateMRT = mRendererDevice->CreatePipelineState(pipelineStateDesc);
    if (!mPipelineStateMRT)
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
    bufferDesc.mode = BufferMode::Static;
    bufferDesc.size = sizeof(vbData);
    bufferDesc.initialData = vbData;
    mVertexBuffer = mRendererDevice->CreateBuffer(bufferDesc);
    if (!mVertexBuffer)
        return false;

    bufferDesc.type = BufferType::Index;
    bufferDesc.mode = BufferMode::Static;
    bufferDesc.size = sizeof(ibData);
    bufferDesc.initialData = ibData;
    mIndexBuffer = mRendererDevice->CreateBuffer(bufferDesc);
    if (!mIndexBuffer)
        return false;

    bufferDesc.type = BufferType::Constant;
    bufferDesc.mode = BufferMode::Volatile;
    bufferDesc.size = sizeof(VertexCBuffer);
    bufferDesc.initialData = nullptr;
    mConstantBuffer = mRendererDevice->CreateBuffer(bufferDesc);
    if (!mConstantBuffer)
        return false;

    return true;
}

bool RenderTargetsScene::CreateRenderTarget(bool withDepthBuffer, bool multipleRT, bool withMSAA)
{
    TextureDesc texDesc;
    texDesc.type = TextureType::Texture2D;
    texDesc.mode = BufferMode::GPUOnly;
    texDesc.width = static_cast<uint16>(WINDOW_WIDTH / 2);
    texDesc.height = static_cast<uint16>(WINDOW_HEIGHT / 2);
    texDesc.mipmaps = 1;
    texDesc.samplesNum = withMSAA ? MULTISAMPLE_SAMPLES : 1;

    // render target texture
    texDesc.format = ElementFormat::R8G8B8A8_U_Norm;
    texDesc.binding = NFE_RENDERER_TEXTURE_BIND_RENDERTARGET | NFE_RENDERER_TEXTURE_BIND_SHADER;
    texDesc.debugName = "RenderTargetsScene::mRenderTargetTexture[0]";
    mRenderTargetTextures[0] = mRendererDevice->CreateTexture(texDesc);
    if (!mRenderTargetTextures[0])
        return false;

    // create and fill binding set instance
    {
        mPSBindingInstancePrimary = mRendererDevice->CreateResourceBindingInstance(mPSBindingSet);
        if (!mPSBindingInstancePrimary)
            return false;
        if (!mPSBindingInstancePrimary->WriteTextureView(0, mRenderTargetTextures[0]))
            return false;
    }

    if (multipleRT)
    {
        texDesc.debugName = "RenderTargetsScene::mRenderTargetTexture[1]";
        mRenderTargetTextures[1] = mRendererDevice->CreateTexture(texDesc);
        if (!mRenderTargetTextures[1])
            return false;

        // create and fill binding set instance
        mPSBindingInstanceSecondary = mRendererDevice->CreateResourceBindingInstance(mPSBindingSet);
        if (!mPSBindingInstanceSecondary)
            return false;
        if (!mPSBindingInstanceSecondary->WriteTextureView(0, mRenderTargetTextures[1]))
            return false;
    }

    // render target's depth buffer
    if (withDepthBuffer)
    {
        texDesc.binding = NFE_RENDERER_TEXTURE_BIND_DEPTH | NFE_RENDERER_TEXTURE_BIND_SHADER;
        texDesc.depthBufferFormat = DepthBufferFormat::Depth16;
        texDesc.debugName = "RenderTargetsScene::mDepthBuffer";
        mDepthBuffer = mRendererDevice->CreateTexture(texDesc);
        if (!mDepthBuffer)
            return false;

        // create and fill binding set instance
        mPSBindingInstanceDepth = mRendererDevice->CreateResourceBindingInstance(mPSBindingSet);
        if (!mPSBindingInstanceDepth)
            return false;
        if (!mPSBindingInstanceDepth->WriteTextureView(0, mDepthBuffer))
            return false;
    }

    RenderTargetElement rtTargets[2];
    rtTargets[0].texture = mRenderTargetTextures[0];
    rtTargets[1].texture = mRenderTargetTextures[1];
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = multipleRT ? 2 : 1;
    rtDesc.targets = rtTargets;
    rtDesc.depthBuffer = withDepthBuffer ? mDepthBuffer : nullptr;
    rtDesc.debugName = "RenderTargetsScene::mRenderTarget";
    mRenderTarget = mRendererDevice->CreateRenderTarget(rtDesc);
    if (!mRenderTarget)
        return false;

    return true;
}

bool RenderTargetsScene::CreateShaders(bool multipleRT, bool withMSAA)
{
    // Common Vertex Shader

    ShaderMacro vsMacro[] = { { "USE_CBUFFER", "1" } };
    std::string vsPath = gShaderPathPrefix + "TestVS" + gShaderPathExt;
    mVertexShader = CompileShader(vsPath.c_str(), ShaderType::Vertex, vsMacro, 1);
    if (!mVertexShader)
        return false;

    // Pixel Shader for 3D cube rendering

    ShaderMacro psMacroRT[] = { { "TARGETS", multipleRT ? "2" : "1" } };
    std::string psPath = gShaderPathPrefix + "RenderTargetPS" + gShaderPathExt;
    mRTPixelShader = CompileShader(psPath.c_str(), ShaderType::Pixel, psMacroRT, 1);
    if (!mRTPixelShader)
        return false;

    char samplesNumStr[8];
    snprintf(samplesNumStr, 8, "%i", withMSAA ? MULTISAMPLE_SAMPLES : 1);

    // Pixel Shader for primary render target preview

    ShaderMacro psMacrosNormal[] = { { "MODE", "0" }, { "SAMPLES_NUM", samplesNumStr } };
    psPath = gShaderPathPrefix + "PostProcessPS" + gShaderPathExt;
    mPrimaryTargetPixelShader = CompileShader(psPath.c_str(), ShaderType::Pixel, psMacrosNormal, 2);
    if (!mPrimaryTargetPixelShader)
        return false;

    // Pixel Shader for depthbuffer preview

    ShaderMacro psMacroDepth[] = { { "MODE", "1" }, { "SAMPLES_NUM", samplesNumStr } };
    psPath = gShaderPathPrefix + "PostProcessPS" + gShaderPathExt;
    mDepthPixelShader = CompileShader(psPath.c_str(), ShaderType::Pixel, psMacroDepth, 2);
    if (!mDepthPixelShader)
        return false;

    // Pixel Shader for secondary render target preview

    ShaderMacro psMacroSecondary[] = { { "MODE", "2" }, { "SAMPLES_NUM", samplesNumStr } };
    psPath = gShaderPathPrefix + "PostProcessPS" + gShaderPathExt;
    mSecondTargetPixelShader = CompileShader(psPath.c_str(), ShaderType::Pixel, psMacroSecondary, 2);
    if (!mSecondTargetPixelShader)
        return false;


    /**
     * We can obtain slot only for one shader program, because we use the same shaders
     * with hardcoded binding slots.
     */
    int cbufferSlot = mVertexShader->GetResourceSlotByName("TestCBuffer");
    if (cbufferSlot < 0)
        return false;
    int textureSlot = mPrimaryTargetPixelShader->GetResourceSlotByName("gTexture");
    if (textureSlot < 0)
        return false;

    // sampler to be used with our textures
    SamplerDesc samplerDesc;
    mSampler = mRendererDevice->CreateSampler(samplerDesc);
    if (!mSampler)
        return false;

    // define cbuffer binding for VS
    VolatileCBufferBinding cbufferDesc(ShaderType::Vertex, ShaderResourceType::CBuffer, cbufferSlot,
                                       sizeof(VertexCBuffer));

    // create binding set for pixel shader
    ResourceBindingDesc pixelShaderBinding(ShaderResourceType::Texture,
                                           textureSlot,
                                           mSampler);
    mPSBindingSet = mRendererDevice->CreateResourceBindingSet(ResourceBindingSetDesc(&pixelShaderBinding, 1, ShaderType::Pixel));
    if (!mPSBindingSet)
        return false;

    // create binding layout
    ResourceBindingSetPtr bindingSets[] = { mPSBindingSet };
    mResBindingLayout = mRendererDevice->CreateResourceBindingLayout(ResourceBindingLayoutDesc(bindingSets, 1, &cbufferDesc, 1));
    if (!mResBindingLayout)
        return false;

    return true;
}

bool RenderTargetsScene::CreateSubSceneNoDepthBuffer()
{
    if (!CreateShaders())
        return false;
    if (!CreateBasicResources(false, false))
        return false;
    return CreateRenderTarget();
}

bool RenderTargetsScene::CreateSubSceneDepthBuffer()
{
    if (!CreateShaders())
        return false;
    if (!CreateBasicResources(false, true))
        return false;
    return CreateRenderTarget(true);
}

bool RenderTargetsScene::CreateSubSceneMRT()
{
    if (!CreateShaders(true))
        return false;
    if (!CreateBasicResources(true, true))
        return false;
    return CreateRenderTarget(true, true);
}

bool RenderTargetsScene::CreateSubSceneMRTandMSAA()
{
    if (!CreateShaders(true, true))
        return false;
    if (!CreateBasicResources(true, true, true))
        return false;
    return CreateRenderTarget(true, true, true);
}

bool RenderTargetsScene::OnInit(void* winHandle)
{
    // create backbuffer connected with the window
    BackbufferDesc bbDesc;
    bbDesc.width = WINDOW_WIDTH;
    bbDesc.height = WINDOW_HEIGHT;
    bbDesc.format = mBackbufferFormat;
    bbDesc.windowHandle = winHandle;
    bbDesc.vSync = false;
    bbDesc.debugName = "RenderTargetsScene::mWindowBackbuffer";
    mWindowBackbuffer = mRendererDevice->CreateBackbuffer(bbDesc);
    if (!mWindowBackbuffer)
        return false;

    return true;
}

bool RenderTargetsScene::OnSwitchSubscene()
{
    mAngle = 0.0f;
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

    mCommandBuffer->Begin();

    int stride = 9 * sizeof(float);
    int offset = 0;
    BufferPtr vb = mVertexBuffer;
    mCommandBuffer->SetVertexBuffers(1, &vb, &stride, &offset);
    mCommandBuffer->SetIndexBuffer(mIndexBuffer, IndexBufferFormat::Uint16);
    mCommandBuffer->SetResourceBindingLayout(mResBindingLayout);

    BufferPtr cb = mConstantBuffer;
    mCommandBuffer->BindVolatileCBuffer(0, cb);

    // render cube to a texture
    {
        mCommandBuffer->SetViewport(0.0f, (float)(WINDOW_WIDTH / 2),
                                    0.0f, (float)(WINDOW_HEIGHT / 2), 0.0f, 1.0f);
        mCommandBuffer->SetScissors(0, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

        VertexCBuffer cbuffer;
        cbuffer.viewMatrix = modelMatrix * viewMatrix * projMatrix;
        mCommandBuffer->WriteBuffer(cb, 0, sizeof(VertexCBuffer), &cbuffer);

        mCommandBuffer->SetRenderTarget(mRenderTarget);
        mCommandBuffer->SetPipelineState(mPipelineStateMRT);

        const Float4 colors[] =
        {
            Float4(0.2f, 0.3f, 0.4f, 1.0f),
            Float4(0.8f, 0.8f, 0.8f, 1.0f),
        };
        mCommandBuffer->Clear(ClearFlagsColor | ClearFlagsDepth, GetCurrentSubSceneNumber() > 1 ? 2 : 1,
                              nullptr, colors, 1.0f);

        mCommandBuffer->DrawIndexed(2 * 6 * 3);

        // unbind render target
        mCommandBuffer->SetRenderTarget(nullptr);
    }

    // begin rendering to the window
    mCommandBuffer->SetPipelineState(mPrimaryTargetPipelineState);
    mCommandBuffer->BindResources(0, mPSBindingInstancePrimary);
    mCommandBuffer->SetRenderTarget(mWindowRenderTarget);
    mCommandBuffer->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);
    mCommandBuffer->SetScissors(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    const Float4 color(0.0f, 0.0f, 0.0f, 1.0f);
    mCommandBuffer->Clear(ClearFlagsColor, 1, nullptr, &color);

    // show primary render target texture with simple effect (blur)
    {
        VertexCBuffer cbuffer;
        cbuffer.viewMatrix = MatrixScaling(Vector(0.5f, 0.5f, 0.5f, 1.0f)) *
                             MatrixTranslation3(Vector(-0.5f, 0.5f, 0.5f));
        mCommandBuffer->WriteBuffer(cb, 0, sizeof(VertexCBuffer), &cbuffer);

        mCommandBuffer->DrawIndexed(2 * 3,      // 2 triangles
                                    1,          // no instancing
                                    2 * 6 * 3); // omit cube vertices
    }

    // show depth-buffer
    if (GetCurrentSubSceneNumber() > 0)
    {
        VertexCBuffer cbuffer;
        cbuffer.viewMatrix = MatrixScaling(Vector(0.5f, 0.5f, 0.5f, 1.0f)) *
            MatrixTranslation3(Vector(0.5f, 0.5f, 0.5f));
        mCommandBuffer->WriteBuffer(cb, 0, sizeof(VertexCBuffer), &cbuffer);

        mCommandBuffer->SetPipelineState(mDepthPipelineState);
        mCommandBuffer->BindResources(0, mPSBindingInstanceDepth);
        mCommandBuffer->DrawIndexed(2 * 3,      // 2 triangles
                                    1,          // no instancing
                                    2 * 6 * 3); // omit cube vertices
    }

    // show secondary render target texture
    if (GetCurrentSubSceneNumber() > 1)
    {
        VertexCBuffer cbuffer;
        cbuffer.viewMatrix = MatrixScaling(Vector(0.5f, 0.5f, 0.5f, 1.0f)) *
            MatrixTranslation3(Vector(-0.5f, -0.5f, 0.5f));
        mCommandBuffer->WriteBuffer(cb, 0, sizeof(VertexCBuffer), &cbuffer);

        mCommandBuffer->SetPipelineState(mSecondTargetPipelineState);
        mCommandBuffer->BindResources(0, mPSBindingInstanceSecondary);
        mCommandBuffer->DrawIndexed(2 * 3,      // 2 triangles
                                    1,          // no instancing
                                    2 * 6 * 3); // omit cube vertices
    }

    // unbind texture from pixel shader, because we will be rendering to it in the next frame
    mCommandBuffer->BindResources(0, nullptr);

    CommandListID commandList = mCommandBuffer->Finish();
    mRendererDevice->Execute(commandList);
    mWindowBackbuffer->Present();
    mRendererDevice->FinishFrame();
}

void RenderTargetsScene::ReleaseSubsceneResources()
{
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
    mPipelineStateMRT.reset();
    mPrimaryTargetPipelineState.reset();
    mDepthPipelineState.reset();
    mSecondTargetPipelineState.reset();
    mConstantBuffer.reset();
    mVertexBuffer.reset();
    mIndexBuffer.reset();
    mVertexLayout.reset();
    mSampler.reset();


    mPSBindingInstancePrimary.reset();
    mPSBindingInstanceDepth.reset();
    mPSBindingInstanceSecondary.reset();
    mResBindingLayout.reset();
    mPSBindingSet.reset();
}

void RenderTargetsScene::Release()
{
    ReleaseSubsceneResources();
    mWindowBackbuffer.reset();
    mCommandBuffer = nullptr;
    mRendererDevice = nullptr;
}
