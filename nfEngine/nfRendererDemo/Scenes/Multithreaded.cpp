/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Source file with multithreaded test scene definition
 */

#include "../PCH.hpp"

#include "Multithreaded.hpp"
#include "../Common.hpp"

#include "nfCommon/Math/Math.hpp"
#include "nfCommon/Math/Matrix4.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/System/Assertion.hpp"

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

} // namespace

/// Helper creators for the Scene

bool MultithreadedScene::CreateCommandRecorders()
{
    size_t num = mThreadPool.GetThreadsNumber();
    mCommandRecorders.resize(num);

    for (size_t i = 0; i < num; ++i)
    {
        mCommandRecorders[i] = mRendererDevice->CreateCommandRecorder();
        if (!mCommandRecorders[i])
            return false;
    }

    return true;
}

bool MultithreadedScene::CreateShaders(BufferMode cbufferMode)
{
    mCBufferSlot = -1;

    ShaderMacro vsMacro[] = { { "USE_CBUFFER", "1" } };
    const Common::String vsPath = gShaderPathPrefix + "TestVS" + gShaderPathExt;
    mVertexShader = CompileShader(vsPath.Str(), ShaderType::Vertex, vsMacro, 1);
    if (!mVertexShader)
        return false;

    ShaderMacro psMacro[] = { { "USE_TEXTURE", "0" } };
    const Common::String psPath = gShaderPathPrefix + "TestPS" + gShaderPathExt;
    mPixelShader = CompileShader(psPath.Str(), ShaderType::Pixel, psMacro, 1);
    if (!mPixelShader)
        return false;

    int numBindingSets = 0;
    ResourceBindingSetPtr bindingSets[2];

    // create binding set

    mCBufferSlot = mVertexShader->GetResourceSlotByName("TestCBuffer");
    if (mCBufferSlot < 0)
        return false;

    if (cbufferMode == BufferMode::Static || cbufferMode == BufferMode::Dynamic)
    {
        ResourceBindingDesc vertexShaderBinding(ShaderResourceType::CBuffer, mCBufferSlot);
        mVSBindingSet = mRendererDevice->CreateResourceBindingSet(ResourceBindingSetDesc(&vertexShaderBinding, 1, ShaderType::Vertex));
        if (!mVSBindingSet)
            return false;

        mVSBindingSlot = numBindingSets++;
        bindingSets[mVSBindingSlot] = mVSBindingSet;
    }

    bool useVolatileCBufferBinding = cbufferMode == BufferMode::Volatile;
    VolatileCBufferBinding volatileCBufferBinding(ShaderType::Vertex,
                                                  ShaderResourceType::CBuffer,
                                                  mCBufferSlot, sizeof(VertexCBuffer));

    // create binding layout
    mResBindingLayout = mRendererDevice->CreateResourceBindingLayout(
        ResourceBindingLayoutDesc(bindingSets, numBindingSets, &volatileCBufferBinding, useVolatileCBufferBinding ? 1 : 0));
    if (!mResBindingLayout)
        return false;

    return true;
}

bool MultithreadedScene::CreateVertexBuffer()
{
    // create vertex buffer
    float vbDataExtra[] =
    {
        /// Vertex structure: pos.xyz, texCoord.uv, color.rgba

        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    };

    BufferDesc vbDesc;
    vbDesc.type = BufferType::Vertex;
    vbDesc.mode = BufferMode::Static;
    vbDesc.size = sizeof(vbDataExtra);
    vbDesc.initialData = vbDataExtra;
    mVertexBuffer = mRendererDevice->CreateBuffer(vbDesc);
    if (!mVertexBuffer)
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
    pipelineStateDesc.pixelShader = mPixelShader;
    pipelineStateDesc.blendState.independent = false;
    pipelineStateDesc.blendState.rtDescs[0].enable = true;
    pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
    pipelineStateDesc.vertexLayout = mVertexLayout;
    pipelineStateDesc.resBindingLayout = mResBindingLayout;
    mPipelineState = mRendererDevice->CreatePipelineState(pipelineStateDesc);
    if (!mPipelineState)
        return false;

    return true;
}

bool MultithreadedScene::CreateIndexBuffer()
{
    // create index buffer
    uint16 ibData[] =
    {
        0, 1, 2,
        1, 3, 2
    };

    BufferDesc ibDesc;
    ibDesc.type = BufferType::Index;
    ibDesc.mode = BufferMode::Static;
    ibDesc.size = sizeof(ibData);
    ibDesc.initialData = ibData;
    mIndexBuffer = mRendererDevice->CreateBuffer(ibDesc);
    if (!mIndexBuffer)
        return false;

    return true;
}

bool MultithreadedScene::CreateConstantBuffer(BufferMode cbufferMode)
{
    const Matrix4 rotMatrix = Matrix4::MakeRotationNormal(Vector4(0.0f, 0.0f, 1.0f), Constants::pi<float>);
    mAngle = 0.0f;
    mCBufferMode = cbufferMode;

    BufferDesc cbufferDesc;
    cbufferDesc.type = BufferType::Constant;
    cbufferDesc.mode = cbufferMode;
    cbufferDesc.size = sizeof(VertexCBuffer);

    if (cbufferMode == BufferMode::Static)
    {
        cbufferDesc.initialData = &rotMatrix;
    }

    mConstantBuffer = mRendererDevice->CreateBuffer(cbufferDesc);
    if (!mConstantBuffer)
        return false;

    if (cbufferMode == BufferMode::Static || cbufferMode == BufferMode::Dynamic)
    {
        // create and fill binding set instance
        mVSBindingInstance = mRendererDevice->CreateResourceBindingInstance(mVSBindingSet);
        if (!mVSBindingInstance)
            return false;
        if (!mVSBindingInstance->WriteCBufferView(0, mConstantBuffer))
            return false;
    }

    return true;
}


/////////////////
/// Subscenes ///
/////////////////

// Basic initialization, additionally to RT & BackBuffer shaders are compiled
// Empty window should be visible
bool MultithreadedScene::CreateSubSceneEmpty()
{
    mGridSize = 0;

    if (!CreateCommandRecorders())
        return false;

    return CreateShaders(BufferMode::Static);
}

bool MultithreadedScene::CreateSubSceneNormal(BufferMode cbufferMode, int gridSize)
{
    mGridSize = gridSize;

    if (!CreateCommandRecorders())
        return false;

    if (!CreateShaders(cbufferMode))
        return false;

    if (!CreateVertexBuffer())
        return false;

    if (!CreateIndexBuffer())
        return false;

    if (!CreateConstantBuffer(cbufferMode))
        return false;

    return true;
}

/////////////////////////////////////////////////
/// MultithreadedScene methods and virtuals overridden ///
/////////////////////////////////////////////////

MultithreadedScene::MultithreadedScene()
    : Scene("Multithreaded")
{
    mCollectedCommandLists.resize(mThreadPool.GetThreadsNumber());

    RegisterSubScene(std::bind(&MultithreadedScene::CreateSubSceneEmpty, this), "Empty");

    RegisterSubScene(std::bind(&MultithreadedScene::CreateSubSceneNormal, this, BufferMode::Dynamic, 1), "1x1, dynamic cbuffer");
    RegisterSubScene(std::bind(&MultithreadedScene::CreateSubSceneNormal, this, BufferMode::Dynamic, 4), "4x4, dynamic cbuffer");
    RegisterSubScene(std::bind(&MultithreadedScene::CreateSubSceneNormal, this, BufferMode::Dynamic, 10), "10x10, dynamic cbuffer");
    RegisterSubScene(std::bind(&MultithreadedScene::CreateSubSceneNormal, this, BufferMode::Dynamic, 30), "30x30, dynamic cbuffer");

    RegisterSubScene(std::bind(&MultithreadedScene::CreateSubSceneNormal, this, BufferMode::Volatile, 1), "1x1, volatile cbuffer");
    RegisterSubScene(std::bind(&MultithreadedScene::CreateSubSceneNormal, this, BufferMode::Volatile, 4), "4x4, volatile cbuffer");
    RegisterSubScene(std::bind(&MultithreadedScene::CreateSubSceneNormal, this, BufferMode::Volatile, 10), "10x10, volatile cbuffer");
    RegisterSubScene(std::bind(&MultithreadedScene::CreateSubSceneNormal, this, BufferMode::Volatile, 30), "30x30, volatile cbuffer");
}

MultithreadedScene::~MultithreadedScene()
{
    Release();
}

void MultithreadedScene::ReleaseSubsceneResources()
{
    // clear resources
    mConstantBuffer.Reset();
    mIndexBuffer.Reset();
    mVertexLayout.Reset();
    mVertexBuffer.Reset();
    mPixelShader.Reset();
    mVertexShader.Reset();
    mPipelineState.Reset();

    mVSBindingInstance.Reset();
    mResBindingLayout.Reset();
    mVSBindingSet.Reset();

    mCommandRecorders.clear();
}

bool MultithreadedScene::OnInit(void* winHandle)
{
    // create backbuffer connected with the window
    BackbufferDesc bbDesc;
    bbDesc.width = WINDOW_WIDTH;
    bbDesc.height = WINDOW_HEIGHT;
    bbDesc.format = mBackbufferFormat;
    bbDesc.windowHandle = winHandle;
    bbDesc.vSync = false;
    mWindowBackbuffer = mRendererDevice->CreateBackbuffer(bbDesc);
    if (!mWindowBackbuffer)
        return false;

    // create rendertarget that will render to the window's backbuffer
    RenderTargetElement rtTarget;
    rtTarget.texture = mWindowBackbuffer;
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;
    mWindowRenderTarget = mRendererDevice->CreateRenderTarget(rtDesc);
    if (!mWindowRenderTarget)
        return false;

    return true;
}

void MultithreadedScene::DrawTask(const Common::TaskContext& ctx, int i, int j)
{
    // NOTE:
    // this is extremely inefficient to draw single square in a separate task
    // and submit it as a single command list, but it's not performance test

    NFE_ASSERT(ctx.threadId < mCommandRecorders.size(), "Invalid thread ID");
    const CommandRecorderPtr& recorder = mCommandRecorders[ctx.threadId];

    // reset bound resources and set them once again
    recorder->Begin();
    recorder->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);
    recorder->SetScissors(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    recorder->SetRenderTarget(mWindowRenderTarget);

    uint32 stride = 9 * sizeof(float);
    uint32 offset = 0;
    const BufferPtr& vb = mVertexBuffer;
    recorder->SetVertexBuffers(1, &vb, &stride, &offset);
    recorder->SetIndexBuffer(mIndexBuffer, IndexBufferFormat::Uint16);

    recorder->SetResourceBindingLayout(mResBindingLayout);
    recorder->SetPipelineState(mPipelineState);

    if (mCBufferMode == BufferMode::Static || mCBufferMode == BufferMode::Dynamic)
        recorder->BindResources(mVSBindingSlot, mVSBindingInstance);
    else if (mCBufferMode == BufferMode::Volatile)
        recorder->BindVolatileCBuffer(0, mConstantBuffer);


    const float scaleCoeff = 1.0f / static_cast<float>(mGridSize);
    if (mConstantBuffer && (mCBufferMode == BufferMode::Dynamic || mCBufferMode == BufferMode::Volatile))
    {
        float xOffset = 2.0f * (static_cast<float>(i) + 0.5f) * scaleCoeff - 1.0f;
        float yOffset = 2.0f * (static_cast<float>(j) + 0.5f) * scaleCoeff - 1.0f;
        const float angle = mAngle + 5.0f * i + 7.0f * j;

        const Matrix4 rotMatrix = Matrix4::MakeRotationNormal(Vector4(0.0f, 0.0f, 1.0f), angle);
        const Matrix4 translationMatrix = Matrix4::MakeTranslation3(Vector4(xOffset, yOffset, 0.0f));
        const Matrix4 scaleMatrix = Matrix4::MakeScaling(Vector4(scaleCoeff, scaleCoeff, 0.0f));

        VertexCBuffer vertexCBufferData;
        vertexCBufferData.viewMatrix = scaleMatrix * rotMatrix * translationMatrix;
        recorder->WriteBuffer(mConstantBuffer, 0, sizeof(VertexCBuffer), &vertexCBufferData);
    }

    // draw
    recorder->DrawIndexed(6, 1);

    CommandListID commandList = recorder->Finish();
    mCollectedCommandLists[ctx.threadId].push_back(commandList);
}

void MultithreadedScene::Draw(float dt)
{
    // apply rotation
    mAngle += 2.0f * dt;
    if (mAngle > 2.0f * Constants::pi<float>)
        mAngle -= 2.0f * Constants::pi<float>;

    // clear only once
    CommandListID clearCommandList;
    {
        mCommandBuffer->Begin();
        mCommandBuffer->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);
        mCommandBuffer->SetScissors(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        mCommandBuffer->SetRenderTarget(mWindowRenderTarget);

        // clear target
        const Float4 color(0.0f, 0.0f, 0.0f, 1.0f);
        mCommandBuffer->Clear(ClearFlagsColor, 1, nullptr, &color);

        clearCommandList = mCommandBuffer->Finish();
    }

    for (int i = 0; i < mGridSize; ++i)
    {
        for (int j = 0; j < mGridSize; ++j)
        {
            using namespace std::placeholders;
            mThreadPool.CreateTask(std::bind(&MultithreadedScene::DrawTask, this, _1, i, j));
        }
    }

    mRendererDevice->Execute(clearCommandList);

    // TODO execute some more command lists while recording

    mThreadPool.WaitForAllTasks();

    // execute recorded command lists
    for (CollectedCommandLists& collectedCommandLists : mCollectedCommandLists)
    {
        for (CommandListID commandList : collectedCommandLists)
        {
            mRendererDevice->Execute(commandList);
        }

        collectedCommandLists.clear();
    }

    mWindowBackbuffer->Present();
    mRendererDevice->FinishFrame();
}

void MultithreadedScene::Release()
{
    ReleaseSubsceneResources();
    mWindowRenderTarget.Reset();
    mWindowBackbuffer.Reset();
    mCommandBuffer.Reset();
    mRendererDevice = nullptr;
}
