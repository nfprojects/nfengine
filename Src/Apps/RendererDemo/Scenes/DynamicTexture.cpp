/**
 * @file
 * @author  Witek902
 * @brief   Definition of Dynamic Texture scene
 */

#include "PCH.hpp"

#include "Scene.hpp"

#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Common/Math/Math.hpp"
#include "Engine/Common/Math/Vec4fU.hpp"
#include "Engine/Common/Reflection/ReflectionClassDefine.hpp"
#include "Engine/Common/Math/Random.hpp"


class DynamicTextureScene : public Scene
{
    NFE_DECLARE_POLYMORPHIC_CLASS(DynamicTextureScene)

    NFE::Renderer::TexturePtr mTexture;
    NFE::Renderer::RenderTargetPtr mRenderTarget;

    // using some weird resolution to make sure it works as intended
    static constexpr NFE::uint32 TexRegionWidth = 191;
    static constexpr NFE::uint32 TexRegionHeight = 233;

    bool mUseCopyQueue = false;

    NFE::Math::Random mRandom;
    NFE::Common::DynArray<NFE::uint32> mTextureData; // assumes RGBA 8-bit format

    float mTime = 0.0f;

    // Releases only subscene-related resources. Backbuffer, RT and BlendState stay intact.
    void ReleaseSubsceneResources() override;

    // Subscenes
    bool CreateSubSceneSimple(bool useCopyQueue);

public:
    DynamicTextureScene();
    ~DynamicTextureScene();

    bool OnInit(void* winHandle) override;
    void Draw(float dt) override;
    void Release() override;
};


NFE_DEFINE_POLYMORPHIC_CLASS(DynamicTextureScene)
    NFE_CLASS_PARENT(Scene)
NFE_END_DEFINE_CLASS()


using namespace NFE;
using namespace NFE::Math;
using namespace NFE::Renderer;

bool DynamicTextureScene::CreateSubSceneSimple(bool useCopyQueue)
{
    TextureDesc textureDesc;
    textureDesc.mode = ResourceAccessMode::GPUOnly;
    textureDesc.format = mBackbufferFormat; // match with backbuffer format, because we copy the data directly
    textureDesc.width = WINDOW_WIDTH;
    textureDesc.height = WINDOW_HEIGHT;
    textureDesc.usage = TextureUsageFlag::RenderTarget;
    mTexture = mRendererDevice->CreateTexture(textureDesc);
    if (!mTexture)
    {
        return false;
    }

    // create rendertarget (for clearing)
    RenderTargetDesc rtDesc;
    rtDesc.targets = { RenderTargetElement(mTexture) };
    mRenderTarget = mRendererDevice->CreateRenderTarget(rtDesc);
    if (!mRenderTarget)
        return false;

    mTextureData.Resize(TexRegionWidth * TexRegionHeight);

    mUseCopyQueue = useCopyQueue;

    return true;
}

DynamicTextureScene::DynamicTextureScene()
    : Scene("DynamicTexture")
{
    RegisterSubScene(std::bind(&DynamicTextureScene::CreateSubSceneSimple, this, false), "Simple");
    RegisterSubScene(std::bind(&DynamicTextureScene::CreateSubSceneSimple, this, true), "CopyQueue");
}

DynamicTextureScene::~DynamicTextureScene()
{
    Release();
}

void DynamicTextureScene::ReleaseSubsceneResources()
{
    Scene::ReleaseSubsceneResources();
}

bool DynamicTextureScene::OnInit(void* winHandle)
{
    // create backbuffer connected with the window
    BackbufferDesc bbDesc;
    bbDesc.width = WINDOW_WIDTH;
    bbDesc.height = WINDOW_HEIGHT;
    bbDesc.format = mBackbufferFormat;
    bbDesc.windowHandle = winHandle;
    bbDesc.commandQueue = mGraphicsQueue;
    mWindowBackbuffer = mRendererDevice->CreateBackbuffer(bbDesc);
    if (!mWindowBackbuffer)
    {
        return false;
    }

    return true;
}

void DynamicTextureScene::Draw(float dt)
{
    mTime += dt * 0.5f;
    if (mTime >= 1.0f)
    {
        mTime -= 1.0f;
    }

    CommandListPtr commandList_CopyToTexture;
    {
        mCommandBuffer->Begin(mUseCopyQueue ? CommandQueueType::Copy : CommandQueueType::Graphics);

        {
            for (uint32 i = 0; i < TexRegionWidth * TexRegionHeight; ++i)
            {
                mTextureData[i] = mRandom.GetInt();
            }

            TextureRegion texRegion;
            texRegion.x = 100;
            texRegion.y = 100;
            texRegion.width = TexRegionWidth;
            texRegion.height = TexRegionHeight;

            mCommandBuffer->WriteTexture(mTexture, mTextureData.Data(), &texRegion);
        }

        {
            const uint32 value = ((uint32)(mTime * 255.0f)) << 16;
            for (uint32 j = 0; j < TexRegionHeight; ++j)
            {
                for (uint32 i = 0; i < TexRegionWidth; ++i)
                {
                    mTextureData[i + TexRegionWidth * j] = (i << 8) | j | value;
                }
            }

            TextureRegion texRegion;
            texRegion.x = 300;
            texRegion.y = 100;
            texRegion.width = TexRegionWidth;
            texRegion.height = TexRegionHeight;

            mCommandBuffer->WriteTexture(mTexture, mTextureData.Data(), &texRegion);
        }

        if (mUseCopyQueue)
        {
            mCommandBuffer->HintTargetCommandQueueType(mTexture, CommandQueueType::Graphics);
        }

        commandList_CopyToTexture = mCommandBuffer->Finish();
    }

    CommandListPtr commandList_CopyToBackbuffer;
    {
        mCommandBuffer->Begin(CommandQueueType::Graphics);

        // copy result to backbuffer
        mCommandBuffer->CopyTexture(mTexture, mWindowBackbuffer);

        commandList_CopyToBackbuffer = mCommandBuffer->Finish();
    }

    CommandListPtr commandList_Clear;
    {
        mCommandBuffer->Begin(CommandQueueType::Graphics);

        mCommandBuffer->SetRenderTarget(mRenderTarget);

        // clear target
        const Vec4fU color(0.0f, 0.0f, 0.0f, 1.0f);
        mCommandBuffer->Clear(ClearFlagsColor, 1, nullptr, &color);

        if (mUseCopyQueue)
        {
            mCommandBuffer->HintTargetCommandQueueType(mTexture, CommandQueueType::Copy);
        }

        commandList_Clear = mCommandBuffer->Finish();
    }

    if (mUseCopyQueue)
    {
        mGraphicsQueue->Execute(commandList_Clear);
        FencePtr clearFence = mGraphicsQueue->Signal(FenceFlag_GpuWaitable);

        mCopyQueue->Wait(clearFence);

        mCopyQueue->Execute(commandList_CopyToTexture);
        FencePtr copyFence = mCopyQueue->Signal(FenceFlag_GpuWaitable);

        mGraphicsQueue->Wait(copyFence);
    }
    else
    {
        mGraphicsQueue->Execute(commandList_Clear);
        mGraphicsQueue->Execute(commandList_CopyToTexture);
    }

    mGraphicsQueue->Execute(commandList_CopyToBackbuffer);

    mWindowBackbuffer->Present();
    mRendererDevice->FinishFrame();
}

void DynamicTextureScene::Release()
{
    ReleaseSubsceneResources();

    mRenderTarget.Reset();
    mWindowBackbuffer.Reset();
    mCommandBuffer.Reset();
    mRendererDevice = nullptr;
}
