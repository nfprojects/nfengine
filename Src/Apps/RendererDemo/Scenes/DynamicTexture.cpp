/**
 * @file
 * @author  Witek902
 * @brief   Definition of Dynamic Texture scene
 */

#include "PCH.hpp"

#include "DynamicTexture.hpp"
#include "../Common.hpp"

#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Common/Math/Math.hpp"
#include "Engine/Common/Math/Vec4fU.hpp"

#include <vector>
#include <functional>


using namespace NFE;
using namespace NFE::Math;
using namespace NFE::Renderer;

bool DynamicTextureScene::CreateSubSceneSimple(bool useCopyQueue)
{
    TextureDesc textureDesc;
    textureDesc.mode = ResourceAccessMode::GPUOnly;
    textureDesc.binding = 0;
    textureDesc.format = mBackbufferFormat; // match with backbuffer format, because we copy the data directly
    textureDesc.width = WINDOW_WIDTH;
    textureDesc.height = WINDOW_HEIGHT;
    textureDesc.binding = NFE_RENDERER_TEXTURE_BIND_RENDERTARGET;
    mTexture = mRendererDevice->CreateTexture(textureDesc);
    if (!mTexture)
    {
        return false;
    }

    // create rendertarget (for clearing)
    RenderTargetElement rtTarget;
    rtTarget.texture = mTexture;
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;
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
