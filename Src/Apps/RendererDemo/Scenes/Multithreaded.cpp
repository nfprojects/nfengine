/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Source file with multithreaded test scene definition
 */

#include "../PCH.hpp"

#include "Scene.hpp"

#include "Engine/Common/Math/Math.hpp"
#include "Engine/Common/Math/Matrix4.hpp"
#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Common/System/Assertion.hpp"
#include "Engine/Common/Utils/ThreadPool.hpp"
#include "Engine/Common/Utils/Waitable.hpp"
#include "Engine/Common/Utils/TaskBuilder.hpp"
#include "Engine/Renderers/RendererCommon/Fence.hpp"
#include "Engine/Common/Reflection/ReflectionClassDefine.hpp"


class MultithreadedScene : public Scene
{
    NFE_DECLARE_POLYMORPHIC_CLASS(MultithreadedScene)

    using CollectedCommandLists = NFE::Common::DynArray<NFE::Renderer::CommandListPtr>;

    NFE::Common::DynArray<CollectedCommandLists> mCollectedCommandLists; // separate for each thread
    NFE::Common::DynArray<NFE::Renderer::CommandRecorderPtr> mCommandRecorders;

    NFE::Renderer::RenderTargetPtr mWindowRenderTarget;
    NFE::Renderer::PipelineStatePtr mPipelineState;
    NFE::Renderer::ShaderPtr mVertexShader;
    NFE::Renderer::ShaderPtr mPixelShader;
    NFE::Renderer::BufferPtr mVertexBuffer;
    NFE::Renderer::VertexLayoutPtr mVertexLayout;
    NFE::Renderer::BufferPtr mIndexBuffer;
    NFE::Renderer::BufferPtr mConstantBuffer;

    int mCBufferSlot;

    // Used for objects rotation in Constant Buffer scenes and onward
    float mAngle;

    // how many instances will be drawn?
    int mGridSize;

    // cbuffer mode
    NFE::Renderer::ResourceAccessMode mCBufferMode;

    // Releases only subscene-related resources. Backbuffer, RT and BlendState stay intact.
    void ReleaseSubsceneResources() override;

    // Resource creators for subscenes
    bool CreateCommandRecorders();
    bool CreateShaders();
    bool CreateVertexBuffer();
    bool CreateIndexBuffer();
    bool CreatePipelineState(NFE::Renderer::ResourceAccessMode cbufferMode);
    bool CreateConstantBuffer(NFE::Renderer::ResourceAccessMode cbufferMode);

    // Subscenes
    bool CreateSubSceneEmpty();
    bool CreateSubSceneNormal(NFE::Renderer::ResourceAccessMode cbufferMode, int gridSize);

    // drawing task (will be called from thread pool)
    void DrawTask(const NFE::Common::TaskContext& ctx, int i, int j);

public:
    MultithreadedScene();
    ~MultithreadedScene();

    bool OnInit(void* winHandle) override;
    void Draw(float dt) override;
    void Release() override;
};


NFE_DEFINE_POLYMORPHIC_CLASS(MultithreadedScene)
    NFE_CLASS_PARENT(Scene)
NFE_END_DEFINE_CLASS()


using namespace NFE;
using namespace NFE::Math;
using namespace NFE::Common;
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
    uint32 num = ThreadPool::GetInstance().GetNumThreads();
    mCommandRecorders.Resize(num);

    for (uint32 i = 0; i < num; ++i)
    {
        mCommandRecorders[i] = mRendererDevice->CreateCommandRecorder();
        if (!mCommandRecorders[i])
            return false;
    }

    return true;
}

bool MultithreadedScene::CreateShaders()
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

    mCBufferSlot = mVertexShader->GetResourceSlotByName("TestCBuffer");
    if (mCBufferSlot < 0)
        return false;

    return true;
}

bool MultithreadedScene::CreateVertexBuffer()
{
    // create vertex buffer
    float vbData[] =
    {
        /// Vertex structure: pos.xyz, texCoord.uv, color.rgba

        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    };

    BufferDesc vbDesc;
    vbDesc.size = sizeof(vbData);
    vbDesc.usage = BufferUsageFlag::VertexBuffer;
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
    ibDesc.size = sizeof(ibData);
    ibDesc.usage = BufferUsageFlag::IndexBuffer;
    mIndexBuffer = mRendererDevice->CreateBuffer(ibDesc);
    if (!mIndexBuffer)
        return false;

    // upload buffer data
    {
        mCommandBuffer->Begin(CommandQueueType::Copy);
        mCommandBuffer->WriteBuffer(mIndexBuffer, 0, sizeof(ibData), ibData);
        mCopyQueue->Execute(mCommandBuffer->Finish());
        mCopyQueue->Signal()->Wait();
    }

    return true;
}

bool MultithreadedScene::CreatePipelineState(ResourceAccessMode)
{
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

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.renderTargetFormats = { mBackbufferFormat };
    pipelineStateDesc.vertexShader = mVertexShader;
    pipelineStateDesc.pixelShader = mPixelShader;
    pipelineStateDesc.blendState.independent = false;
    pipelineStateDesc.blendState.rtDescs[0].enable = true;
    pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
    pipelineStateDesc.vertexLayout = mVertexLayout;
    mPipelineState = mRendererDevice->CreatePipelineState(pipelineStateDesc);
    if (!mPipelineState)
        return false;

    return true;
}

bool MultithreadedScene::CreateConstantBuffer(ResourceAccessMode)
{
    const Matrix4 rotMatrix = Matrix4::MakeRotationNormal(Vec4f(0.0f, 0.0f, 1.0f), Constants::pi<float>);
    mAngle = 0.0f;

    BufferDesc cbufferDesc;
    cbufferDesc.mode = ResourceAccessMode::GPUOnly;
    cbufferDesc.size = sizeof(VertexCBuffer);
    cbufferDesc.usage = BufferUsageFlag::ConstantBuffer;
    mConstantBuffer = mRendererDevice->CreateBuffer(cbufferDesc);
    if (!mConstantBuffer)
        return false;

    // upload buffer data
    {
        mCommandBuffer->Begin(CommandQueueType::Copy);
        mCommandBuffer->WriteBuffer(mConstantBuffer, 0, sizeof(VertexCBuffer), &rotMatrix);
        mCopyQueue->Execute(mCommandBuffer->Finish());
        mCopyQueue->Signal()->Wait();
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

    return CreateShaders();
}

bool MultithreadedScene::CreateSubSceneNormal(ResourceAccessMode cbufferMode, int gridSize)
{
    mGridSize = gridSize;

    if (!CreateCommandRecorders())
        return false;

    if (!CreateShaders())
        return false;

    if (!CreateVertexBuffer())
        return false;

    if (!CreateIndexBuffer())
        return false;

    if (!CreatePipelineState(cbufferMode))
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
    mCollectedCommandLists.Resize(ThreadPool::GetInstance().GetNumThreads());

    RegisterSubScene(std::bind(&MultithreadedScene::CreateSubSceneEmpty, this), "Empty");

    RegisterSubScene(std::bind(&MultithreadedScene::CreateSubSceneNormal, this, ResourceAccessMode::GPUOnly, 1), "1x1, dynamic cbuffer");
    RegisterSubScene(std::bind(&MultithreadedScene::CreateSubSceneNormal, this, ResourceAccessMode::GPUOnly, 4), "4x4, dynamic cbuffer");
    RegisterSubScene(std::bind(&MultithreadedScene::CreateSubSceneNormal, this, ResourceAccessMode::GPUOnly, 10), "10x10, dynamic cbuffer");
}

MultithreadedScene::~MultithreadedScene()
{
    Release();
}

void MultithreadedScene::ReleaseSubsceneResources()
{
    Scene::ReleaseSubsceneResources();

    // clear resources
    mConstantBuffer.Reset();
    mIndexBuffer.Reset();
    mVertexLayout.Reset();
    mVertexBuffer.Reset();
    mPixelShader.Reset();
    mVertexShader.Reset();
    mPipelineState.Reset();

    mCommandRecorders.Clear();
}

bool MultithreadedScene::OnInit(void* winHandle)
{
    if (!Scene::OnInit(winHandle))
    {
        return false;
    }

    // create rendertarget that will render to the window's backbuffer
    RenderTargetDesc rtDesc;
    rtDesc.targets = { RenderTargetElement(mWindowRenderTargetTexture) };
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

    NFE_ASSERT(ctx.threadId < mCommandRecorders.Size(), "Invalid thread ID");
    const CommandRecorderPtr& recorder = mCommandRecorders[ctx.threadId];

    // reset bound resources and set them once again
    recorder->Begin(CommandQueueType::Graphics);
    recorder->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);
    recorder->SetScissors(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    recorder->SetRenderTarget(mWindowRenderTarget);

    uint32 stride = 9 * sizeof(float);
    uint32 offset = 0;
    const BufferPtr& vb = mVertexBuffer;
    recorder->SetVertexBuffers(1, &vb, &stride, &offset);
    recorder->SetIndexBuffer(mIndexBuffer, IndexBufferFormat::Uint16);

    recorder->SetPipelineState(mPipelineState);

    recorder->BindConstantBuffer(ShaderType::Vertex, mCBufferSlot, mConstantBuffer);


    const float scaleCoeff = 1.0f / static_cast<float>(mGridSize);
    if (mConstantBuffer && (mCBufferMode == ResourceAccessMode::GPUOnly))
    {
        float xOffset = 2.0f * (static_cast<float>(i) + 0.5f) * scaleCoeff - 1.0f;
        float yOffset = 2.0f * (static_cast<float>(j) + 0.5f) * scaleCoeff - 1.0f;
        const float angle = mAngle + 5.0f * i + 7.0f * j;

        const Matrix4 rotMatrix = Matrix4::MakeRotationNormal(Vec4f(0.0f, 0.0f, 1.0f), angle);
        const Matrix4 translationMatrix = Matrix4::MakeTranslation(Vec4f(xOffset, yOffset, 0.0f));
        const Matrix4 scaleMatrix = Matrix4::MakeScaling(Vec4f(scaleCoeff, scaleCoeff, 0.0f));

        VertexCBuffer vertexCBufferData;
        vertexCBufferData.viewMatrix = scaleMatrix * rotMatrix * translationMatrix;
        recorder->WriteBuffer(mConstantBuffer, 0, sizeof(VertexCBuffer), &vertexCBufferData);
    }

    // draw
    recorder->DrawIndexed(6, 1);

    CommandListPtr commandList = recorder->Finish();
    mGraphicsQueue->Execute(commandList);
}

void MultithreadedScene::Draw(float dt)
{
    // apply rotation
    mAngle += 2.0f * dt;
    if (mAngle > 2.0f * Constants::pi<float>)
        mAngle -= 2.0f * Constants::pi<float>;

    // clear only once
    {
        mCommandBuffer->Begin(CommandQueueType::Graphics);
        mCommandBuffer->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);
        mCommandBuffer->SetScissors(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        mCommandBuffer->SetRenderTarget(mWindowRenderTarget);

        // clear target
        const Vec4fU color(0.0f, 0.0f, 0.0f, 1.0f);
        mCommandBuffer->Clear(ClearFlagsColor, 1, nullptr, &color);

        CommandListPtr clearCommandList = mCommandBuffer->Finish();
        mGraphicsQueue->Execute(clearCommandList);
    }

    Waitable waitable;
    {
        TaskBuilder builder(waitable);
        for (int i = 0; i < mGridSize; ++i)
        {
            for (int j = 0; j < mGridSize; ++j)
            {
                builder.Task("Draw", [this, i, j] (const TaskContext& ctx)
                {
                    DrawTask(ctx, i, j);
                });
            }
        }
    }

    // copy to back buffer
    mCommandBuffer->Begin(CommandQueueType::Graphics); // TODO should be Copy
    mCommandBuffer->CopyTexture(mWindowRenderTargetTexture, mWindowBackbuffer);
    CommandListPtr commandList = mCommandBuffer->Finish();

    // wait for threads so the commandlists are pushed in right order
    waitable.Wait();

    mGraphicsQueue->Execute(commandList);

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
