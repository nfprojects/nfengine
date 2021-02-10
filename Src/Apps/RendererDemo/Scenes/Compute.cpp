/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definition of Compute scene
 */

#include "../PCH.hpp"

#include "Compute.hpp"
#include "../Common.hpp"

#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Common/Math/Math.hpp"
#include "Engine/Common/Math/Vec4fU.hpp"
#include "Engine/Common/Utils/StringUtils.hpp"
#include "Engine/Renderers/RendererCommon/Fence.hpp"

#include <functional>


using namespace NFE;
using namespace NFE::Math;
using namespace NFE::Renderer;

namespace {

const uint32 THREAD_GROUP_SIZE = 32;

struct CBuffer
{
    uint32 resolution[4];
    Vec4fU resolutionInverse;
};

} // namespace


bool ComputeScene::CreateSubSceneSimple()
{
    mDispatchX = (WINDOW_WIDTH / THREAD_GROUP_SIZE) + ((WINDOW_WIDTH % THREAD_GROUP_SIZE > 0) ? 1 : 0);
    mDispatchY = (WINDOW_HEIGHT / THREAD_GROUP_SIZE) + ((WINDOW_HEIGHT % THREAD_GROUP_SIZE > 0) ? 1 : 0);

    const Common::String threadGroupSizeStr = Common::ToString(THREAD_GROUP_SIZE);
    const Common::String csPath = gShaderPathPrefix + "TestCS" + gShaderPathExt;

    ShaderMacro macros[] =
    {
        { "THREAD_GROUP_SIZE", threadGroupSizeStr.Str() },
        { "FILL_INDIRECT_ARG_BUFFER", "0" },
    };
    mShader_Main = CompileShader(csPath.Str(), ShaderType::Compute, macros, 2);
    if (!mShader_Main)
    {
        return false;
    }

    int mCBufferSlot = mShader_Main->GetResourceSlotByName("gParams");
    if (mCBufferSlot < 0)
    {
        NFE_LOG_ERROR("Slot not found");
        return false;
    }

    int mTextureSlot = mShader_Main->GetResourceSlotByName("gOutputTexture");
    if (mTextureSlot < 0)
    {
        NFE_LOG_ERROR("Slot not found");
        return false;
    }

    // create binding set for pixel shader bindings
    ResourceBindingDesc bindings[] =
    {
        ResourceBindingDesc(ShaderResourceType::CBuffer, mCBufferSlot),
        ResourceBindingDesc(ShaderResourceType::WritableTexture, mTextureSlot),
    };
    mBindingSet = mRendererDevice->CreateResourceBindingSet(ResourceBindingSetDesc(bindings, 2, ShaderType::Compute));
    if (!mBindingSet)
        return false;

    // create binding layout
    const ResourceBindingSetPtr bindingSets[] = { mBindingSet };
    mResBindingLayout = mRendererDevice->CreateResourceBindingLayout(ResourceBindingLayoutDesc(bindingSets, 1));
    if (!mResBindingLayout)
        return false;

    // create pipeline state
    ComputePipelineStateDesc pipelineStateDesc(mShader_Main, mResBindingLayout);
    mPipelineState_Main = mRendererDevice->CreateComputePipelineState(pipelineStateDesc);
    if (!mPipelineState_Main)
        return false;

    // create cbuffer
    CBuffer cubfferData;
    cubfferData.resolution[0] = WINDOW_WIDTH;
    cubfferData.resolution[1] = WINDOW_HEIGHT;
    cubfferData.resolution[2] = cubfferData.resolution[3] = 0;
    cubfferData.resolutionInverse = Vec4fU(1.0f / static_cast<float>(WINDOW_WIDTH),
                                           1.0f / static_cast<float>(WINDOW_HEIGHT), 0.0f, 0.0f);
    BufferDesc cbufferDesc;
    cbufferDesc.size = sizeof(CBuffer);
    cbufferDesc.usage = NFE_RENDERER_BUFFER_USAGE_CONSTANT_BUFFER;
    mConstantBuffer = mRendererDevice->CreateBuffer(cbufferDesc);
    if (!mConstantBuffer)
        return false;

    // upload cbuffer data
    {
        mCommandBuffer->Begin(CommandQueueType::Copy);
        mCommandBuffer->WriteBuffer(mConstantBuffer, 0, sizeof(CBuffer), &cubfferData);
        mCopyQueue->Execute(mCommandBuffer->Finish());
        mCopyQueue->Signal()->Wait();
    }

    // create texture that compute shader will write to
    TextureDesc textureDesc;
    textureDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER_WRITABLE;
    textureDesc.format = mBackbufferFormat; // match with backbuffer format, because we copy the data directly
    textureDesc.width = WINDOW_WIDTH;
    textureDesc.height = WINDOW_HEIGHT;
    textureDesc.debugName = "ComputeScene::mTexture";
    mTexture = mRendererDevice->CreateTexture(textureDesc);
    if (!mTexture)
        return false;

    // create and fill binding set instance
    mBindingInstance = mRendererDevice->CreateResourceBindingInstance(mBindingSet);
    if (!mBindingInstance)
        return false;
    if (!mBindingInstance->SetCBufferView(0, mConstantBuffer))
        return false;
    if (!mBindingInstance->SetWritableTextureView(1, mTexture))
        return false;
    if (!mBindingInstance->Finalize())
        return false;

    mUseIndirectDispatch = false;
    return true;
}

bool ComputeScene::CreateSubSceneIndirect()
{
    if (!CreateSubSceneSimple())
    {
        return false;
    }

    const Common::String threadGroupSizeStr = Common::ToString(THREAD_GROUP_SIZE);
    const Common::String csPath = gShaderPathPrefix + "TestCS" + gShaderPathExt;

    ShaderMacro macros[] =
    {
        { "THREAD_GROUP_SIZE", threadGroupSizeStr.Str() },
        { "FILL_INDIRECT_ARG_BUFFER", "1" },
    };
    mShader_PrepareArg = CompileShader(csPath.Str(), ShaderType::Compute, macros, 2);
    if (!mShader_PrepareArg)
    {
        return false;
    }

    // create pipeline state
    ComputePipelineStateDesc pipelineStateDesc(mShader_PrepareArg, mResBindingLayout);
    mPipelineState_PrepareArg = mRendererDevice->CreateComputePipelineState(pipelineStateDesc);
    if (!mPipelineState_PrepareArg)
        return false;

    BufferDesc cbufferDesc;
    cbufferDesc.size = 3 * sizeof(uint32);
    cbufferDesc.structSize = sizeof(uint32);
    cbufferDesc.usage = NFE_RENDERER_BUFFER_USAGE_WRITABLE_STRUCT_BUFFER;
    mIndirectArgBuffer = mRendererDevice->CreateBuffer(cbufferDesc);
    if (!mIndirectArgBuffer)
    {
        return false;
    }

    mUseIndirectDispatch = true;
    return true;
}

ComputeScene::ComputeScene()
    : Scene("Compute")
{
    RegisterSubScene(std::bind(&ComputeScene::CreateSubSceneSimple, this), "Simple");
    RegisterSubScene(std::bind(&ComputeScene::CreateSubSceneIndirect, this), "Indirect");
}

ComputeScene::~ComputeScene()
{
    Release();
}

void ComputeScene::ReleaseSubsceneResources()
{
    Scene::ReleaseSubsceneResources();

    mBindingInstance.Reset();
    mBindingSet.Reset();
    mResBindingLayout.Reset();

    mTexture.Reset();
    mConstantBuffer.Reset();
    mShader_Main.Reset();
    mShader_PrepareArg.Reset();
    mPipelineState_Main.Reset();
    mPipelineState_PrepareArg.Reset();
}

bool ComputeScene::OnInit(void* winHandle)
{
    // create backbuffer connected with the window
    BackbufferDesc bbDesc;
    bbDesc.width = WINDOW_WIDTH;
    bbDesc.height = WINDOW_HEIGHT;
    bbDesc.format = mBackbufferFormat;
    bbDesc.windowHandle = winHandle;
    bbDesc.vSync = false;
    bbDesc.commandQueue = mGraphicsQueue;
    mWindowBackbuffer = mRendererDevice->CreateBackbuffer(bbDesc);
    if (!mWindowBackbuffer)
        return false;

    return true;
}

void ComputeScene::Draw(float dt)
{
    NFE_UNUSED(dt);

    // reset bound resources and set them once again
    mCommandBuffer->Begin(CommandQueueType::Graphics);

    // bind resources
    mCommandBuffer->SetResourceBindingLayout(PipelineType::Compute, mResBindingLayout);
    mCommandBuffer->BindResources(PipelineType::Compute, 0, mBindingInstance);
    
    // execute compute shader
    if (mUseIndirectDispatch)
    {
        mCommandBuffer->SetComputePipelineState(mPipelineState_PrepareArg);
        mCommandBuffer->BindWritableBuffer(PipelineType::Compute, 0, 1, mIndirectArgBuffer);
        mCommandBuffer->Dispatch();

        mCommandBuffer->SetComputePipelineState(mPipelineState_Main);
        mCommandBuffer->BindWritableTexture(PipelineType::Compute, 0, 1, mTexture);
        mCommandBuffer->DispatchIndirect(mIndirectArgBuffer);
    }
    else
    {   
        mCommandBuffer->SetComputePipelineState(mPipelineState_Main);
        mCommandBuffer->Dispatch(mDispatchX, mDispatchY);
    }

    // copy result to backbuffer
    mCommandBuffer->CopyTexture(mTexture, mWindowBackbuffer);

    CommandListPtr commandList = mCommandBuffer->Finish();
    mGraphicsQueue->Execute(commandList);
    mWindowBackbuffer->Present();
    mRendererDevice->FinishFrame();
}

void ComputeScene::Release()
{
    ReleaseSubsceneResources();

    mWindowBackbuffer.Reset();
    mCommandBuffer.Reset();
    mRendererDevice = nullptr;
}
