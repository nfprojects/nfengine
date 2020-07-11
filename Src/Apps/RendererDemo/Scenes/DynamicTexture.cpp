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

bool DynamicTextureScene::CreateSubSceneSimple()
{
    TextureDesc textureDesc;
    textureDesc.binding = 0;
    textureDesc.format = mBackbufferFormat; // match with backbuffer format, because we copy the data directly
    textureDesc.mode = BufferMode::GPUOnly;
    textureDesc.width = WINDOW_WIDTH;
    textureDesc.height = WINDOW_HEIGHT;
    mTexture = mRendererDevice->CreateTexture(textureDesc);
    if (!mTexture)
    {
        return false;
    }

    mTextureData.Resize(TexRegionWidth * TexRegionHeight);

    return true;
}

DynamicTextureScene::DynamicTextureScene()
    : Scene("DynamicTexture")
{
    RegisterSubScene(std::bind(&DynamicTextureScene::CreateSubSceneSimple, this), "Simple");
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
    bbDesc.vSync = false;
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

    mCommandBuffer->Begin();

    {
        for (uint32 i = 0; i < TexRegionWidth * TexRegionHeight; ++i)
        {
            mTextureData[i] = mRandom.GetInt();
        }

        TextureWriteParams params;
        params.destX = 100;
        params.destY = 100;
        params.width = TexRegionWidth;
        params.height = TexRegionHeight;

        mCommandBuffer->WriteTexture(mTexture, mTextureData.Data(), &params);
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

        TextureWriteParams params;
        params.destX = 300;
        params.destY = 100;
        params.width = TexRegionWidth;
        params.height = TexRegionHeight;

        mCommandBuffer->WriteTexture(mTexture, mTextureData.Data(), &params);
    }

    // copy result to backbuffer
    mCommandBuffer->CopyTexture(mTexture, mWindowBackbuffer);

    CommandListPtr commandList = mCommandBuffer->Finish();
    mRendererDevice->Execute(commandList);
    mWindowBackbuffer->Present();
    mRendererDevice->FinishFrame();
}

void DynamicTextureScene::Release()
{
    ReleaseSubsceneResources();

    mWindowBackbuffer.Reset();
    mCommandBuffer.Reset();
    mRendererDevice = nullptr;
}
