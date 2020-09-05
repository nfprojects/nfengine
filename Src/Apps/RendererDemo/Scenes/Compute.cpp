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

#include <vector>
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

    std::string threadGroupSizeStr = std::to_string(THREAD_GROUP_SIZE);

    ShaderMacro macros[] =
    {
        { "THREADS_X", threadGroupSizeStr.c_str() },
        { "THREADS_Y", threadGroupSizeStr.c_str() },
    };
    const Common::String vsPath = gShaderPathPrefix + "TestCS" + gShaderPathExt;
    mShader = CompileShader(vsPath.Str(), ShaderType::Compute, macros, 2);
    if (!mShader)
        return false;


    int mCBufferSlot = mShader->GetResourceSlotByName("gParams");
    if (mCBufferSlot < 0)
    {
        NFE_LOG_ERROR("Slot not found");
        return false;
    }

    int mTextureSlot = mShader->GetResourceSlotByName("gOutputTexture");
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
    ComputePipelineStateDesc pipelineStateDesc(mShader, mResBindingLayout);
    mPipelineState = mRendererDevice->CreateComputePipelineState(pipelineStateDesc);
    if (!mPipelineState)
        return false;


    // create cbuffer
    CBuffer cubfferData;
    cubfferData.resolution[0] = WINDOW_WIDTH;
    cubfferData.resolution[1] = WINDOW_HEIGHT;
    cubfferData.resolution[2] = cubfferData.resolution[3] = 0;
    cubfferData.resolutionInverse = Vec4fU(1.0f / static_cast<float>(WINDOW_WIDTH),
                                           1.0f / static_cast<float>(WINDOW_HEIGHT), 0.0f, 0.0f);
    BufferDesc cbufferDesc;
    cbufferDesc.type = BufferType::Constant;
    cbufferDesc.mode = BufferMode::Static;
    cbufferDesc.size = sizeof(CBuffer);
    cbufferDesc.initialData = &cubfferData;
    mConstantBuffer = mRendererDevice->CreateBuffer(cbufferDesc);
    if (!mConstantBuffer)
        return false;


    // create texture that compute shader will write to
    TextureDesc textureDesc;
    textureDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER_WRITABLE;
    textureDesc.format = mBackbufferFormat; // match with backbuffer format, because we copy the data directly
    textureDesc.mode = BufferMode::GPUOnly;
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

    return true;
}

ComputeScene::ComputeScene()
    : Scene("Compute")
{
    RegisterSubScene(std::bind(&ComputeScene::CreateSubSceneSimple, this), "Simple");
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
    mShader.Reset();
    mPipelineState.Reset();
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
    mWindowBackbuffer = mRendererDevice->CreateBackbuffer(bbDesc);
    if (!mWindowBackbuffer)
        return false;

    return true;
}

void ComputeScene::Draw(float dt)
{
    NFE_UNUSED(dt);

    // reset bound resources and set them once again
    mCommandBuffer->Begin();

    // bind resources
    mCommandBuffer->SetResourceBindingLayout(PipelineType::Compute, mResBindingLayout);
    mCommandBuffer->BindResources(PipelineType::Compute, 0, mBindingInstance);
    mCommandBuffer->SetComputePipelineState(mPipelineState);

    // execute compute shader
    mCommandBuffer->Dispatch(mDispatchX, mDispatchY);

    // copy result to backbuffer
    mCommandBuffer->CopyTexture(mTexture, mWindowBackbuffer);

    CommandListPtr commandList = mCommandBuffer->Finish();
    mRendererDevice->Execute(commandList);
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
