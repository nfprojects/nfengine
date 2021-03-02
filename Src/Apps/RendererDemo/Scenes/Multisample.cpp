#include "../PCH.hpp"

#include "Multisample.hpp"
#include "../Common.hpp"

#include "Engine/Common/Math/Math.hpp"
#include "Engine/Common/Math/Matrix4.hpp"
#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Renderers/RendererCommon/Fence.hpp"

#include <vector>
#include <functional>


using namespace NFE;
using namespace NFE::Math;
using namespace NFE::Renderer;

namespace {

struct VertexCBuffer
{
    Matrix4 viewMatrix;
};

struct PixelCBuffer
{
    Matrix4 viewMatrix;
};

// TODO: this should be obtained in the runtime
static const int MULTISAMPLE_SAMPLES = 8;

} // namespace

bool MultisampleScene::CreateShaders()
{
    mTextureSlot = -1;
    mCBufferSlot = -1;

    ShaderMacro vsMacro[] = { { "USE_CBUFFER", "0" } };
    const Common::String vsPath = gShaderPathPrefix + "TestVS" + gShaderPathExt;
    mVertexShader = CompileShader(vsPath.Str(), ShaderType::Vertex, vsMacro, 1);
    if (!mVertexShader)
        return false;

    ShaderMacro psMacro[] = { { "USE_TEXTURE", "0" } };
    const Common::String psPath = gShaderPathPrefix + "TestPS" + gShaderPathExt;
    mPixelShader = CompileShader(psPath.Str(), ShaderType::Pixel, psMacro, 1);
    if (!mPixelShader)
        return false;

    // create binding layout
    mResBindingLayout = mRendererDevice->CreateResourceBindingLayout(ResourceBindingLayoutDesc());
    if (!mResBindingLayout)
        return false;

    return true;
}

bool MultisampleScene::CreateVertexBuffer()
{
    // create vertex buffers
    float vbData[] =
    {
        /// Vertex structure: pos.xyz, texCoord.uv, color.rgba

        0.541f, -0.734f, 0.0f, 10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.132f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.1213f, 0.0f, 10.0f, 10.0f, 1.0f, 0.0f, 0.0f, 1.0f,

        -0.5f, -0.132f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.5f, 0.1213f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.6241f, 0.375f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    };

    BufferDesc vbDesc;
    vbDesc.size = sizeof(vbData);
    vbDesc.usage = NFE_RENDERER_BUFFER_USAGE_VERTEX_BUFFER;
    mVertexBuffer = mRendererDevice->CreateBuffer(vbDesc);
    if (!mVertexBuffer)
        return false;

    // upload buffer data
    {
        mCommandBuffer->Begin(CommandQueueType::Copy);
        mCommandBuffer->WriteBuffer(mVertexBuffer, 0, sizeof(vbData), vbData);
        mCopyQueue->Execute(mCommandBuffer->Finish());
        mCopyQueue->Signal()->Wait();
    }

    VertexLayoutElement vertexLayoutElements[] =
    {
        { Format::R32G32B32_Float,       0, 0, false, 0 }, // position
        { Format::R32G32_Float,          12, 0, false, 0 }, // tex-coords
        { Format::R32G32B32A32_Float,    20, 0, false, 0 }, // color
    };

    VertexLayoutDesc vertexLayoutDesc;
    vertexLayoutDesc.elements = vertexLayoutElements;
    vertexLayoutDesc.numElements = 3;
    mVertexLayout = mRendererDevice->CreateVertexLayout(vertexLayoutDesc);
    if (!mVertexLayout)
        return false;

    TextureDesc texDesc;
    texDesc.type = TextureType::Texture2D;
    texDesc.mode = ResourceAccessMode::GPUOnly;
    texDesc.width = WINDOW_WIDTH;
    texDesc.height = WINDOW_HEIGHT;
    texDesc.mipmaps = 1;
    texDesc.samplesNum = MULTISAMPLE_SAMPLES;
    texDesc.format = Format::R8G8B8A8_U_Norm;
    texDesc.binding = NFE_RENDERER_TEXTURE_BIND_RENDERTARGET | NFE_RENDERER_TEXTURE_BIND_SHADER;
    texDesc.defaultColorClearValue[0] = 0.2f;
    texDesc.defaultColorClearValue[1] = 0.3f;
    texDesc.defaultColorClearValue[2] = 0.4f;
    texDesc.defaultColorClearValue[3] = 1.0f;
    texDesc.debugName = "MultisampleScene::mRenderTargetTexture";
    mRenderTargetTexture = mRendererDevice->CreateTexture(texDesc);
    if (!mRenderTargetTexture)
        return false;

    RenderTargetElement rtTargets[2];
    rtTargets[0].texture = mRenderTargetTexture;
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = rtTargets;
    rtDesc.debugName = "MultisampleScene::mRenderTarget";
    mRenderTarget = mRendererDevice->CreateRenderTarget(rtDesc);
    if (!mRenderTarget)
        return false;

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.rtFormats[0] = mBackbufferFormat;
    pipelineStateDesc.vertexShader = mVertexShader;
    pipelineStateDesc.pixelShader = mPixelShader;
    pipelineStateDesc.blendState.independent = false;
    //pipelineStateDesc.blendState.rtDescs[0].enable = true;
    pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
    pipelineStateDesc.vertexLayout = mVertexLayout;
    pipelineStateDesc.resBindingLayout = mResBindingLayout;
    pipelineStateDesc.numSamples = MULTISAMPLE_SAMPLES;
    mPipelineState = mRendererDevice->CreatePipelineState(pipelineStateDesc);
    if (!mPipelineState)
        return false;

    return true;
}

bool MultisampleScene::CreateSubScene()
{
    if (!CreateShaders())
        return false;

    return CreateVertexBuffer();
}

MultisampleScene::MultisampleScene()
    : Scene("Multisample")
{
    RegisterSubScene(std::bind(&MultisampleScene::CreateSubScene, this), "Default");
}

MultisampleScene::~MultisampleScene()
{
    Release();
}

void MultisampleScene::ReleaseSubsceneResources()
{
    Scene::ReleaseSubsceneResources();

    mVertexLayout.Reset();
    mVertexBuffer.Reset();
    mPixelShader.Reset();
    mVertexShader.Reset();
    mPipelineState.Reset();

    mVSBindingInstance.Reset();
    mPSBindingInstance.Reset();
    mResBindingLayout.Reset();
    mVSBindingSet.Reset();
    mPSBindingSet.Reset();
}

bool MultisampleScene::OnInit(void* winHandle)
{
    if (!Scene::OnInit(winHandle))
    {
        return false;
    }

    // create rendertarget that will render to the window's backbuffer
    RenderTargetElement rtTarget;
    rtTarget.texture = mWindowRenderTargetTexture;
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;
    mWindowRenderTarget = mRendererDevice->CreateRenderTarget(rtDesc);
    if (!mWindowRenderTarget)
        return false;

    return true;
}

void MultisampleScene::Draw(float dt)
{
    NFE_UNUSED(dt);

    // reset bound resources and set them once again
    mCommandBuffer->Begin(CommandQueueType::Graphics);
    mCommandBuffer->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);
    mCommandBuffer->SetScissors(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    mCommandBuffer->SetRenderTarget(mRenderTarget);

    uint32 stride = 9 * sizeof(float);
    uint32 offset = 0;

    if (mVertexBuffer)
    {
        const BufferPtr& vb = mVertexBuffer;
        mCommandBuffer->SetVertexBuffers(1, &vb, &stride, &offset);
    }

    if (mResBindingLayout)
    {
        mCommandBuffer->SetResourceBindingLayout(PipelineType::Graphics, mResBindingLayout);
    }

    if (mPipelineState)
    {
        mCommandBuffer->SetPipelineState(mPipelineState);
    }

    // clear target
    const Vec4fU color(0.0f, 1.0f, 0.0f, 1.0f);
    mCommandBuffer->Clear(ClearFlagsColor, 1, nullptr, &color);

    mCommandBuffer->Draw(6, 1);

    mCommandBuffer->CopyTexture(mRenderTargetTexture, mWindowBackbuffer);

    CommandListPtr commandList = mCommandBuffer->Finish();
    mGraphicsQueue->Execute(commandList);
    mWindowBackbuffer->Present();
    mRendererDevice->FinishFrame();
}

void MultisampleScene::Release()
{
    ReleaseSubsceneResources();
    mWindowRenderTarget.Reset();
    mWindowBackbuffer.Reset();
    mCommandBuffer.Reset();
    mRendererDevice = nullptr;
}
