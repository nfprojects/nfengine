#include "PCH.hpp"
#include "Backends.hpp"
#include "DrawTest.hpp"


void DrawTest::BeginTestFrame(uint32 width, uint32 height, size_t numTargets, ElementFormat* formats)
{
    mTestTextureWidth = width;
    mTestTextureHeight = height;

    mTargetTextures.clear();

    mCommandBuffer = gRendererDevice->CreateCommandRecorder();
    ASSERT_FALSE(!mCommandBuffer);

    for (size_t i = 0; i < numTargets; ++i)
    {
        TargetTexture targetTexture;
        targetTexture.format = formats[i];
        targetTexture.textureSize = width * height * GetElementFormatSize(formats[i]);

        TextureDesc texDesc;
        texDesc.binding = NFE_RENDERER_TEXTURE_BIND_RENDERTARGET;
        texDesc.mode = BufferMode::GPUOnly;
        texDesc.format = formats[i];
        texDesc.width = width;
        texDesc.height = height;
        targetTexture.texture = gRendererDevice->CreateTexture(texDesc);
        ASSERT_FALSE(!targetTexture.texture);

        RenderTargetElement rtTarget;
        rtTarget.texture = targetTexture.texture;
        RenderTargetDesc rtDesc;
        rtDesc.numTargets = 1;
        rtDesc.targets = &rtTarget;
        mTestRenderTarget = gRendererDevice->CreateRenderTarget(rtDesc);
        ASSERT_FALSE(!mTestRenderTarget);

        texDesc.binding = 0;
        texDesc.mode = BufferMode::Readback;
        targetTexture.readbackTexture = gRendererDevice->CreateTexture(texDesc);
        ASSERT_FALSE(!targetTexture.readbackTexture);

        mTargetTextures.emplace_back(std::move(targetTexture));
    }

    ASSERT_TRUE(mCommandBuffer->Begin());
    mCommandBuffer->SetRenderTarget(mTestRenderTarget);
    mCommandBuffer->SetScissors(0, 0, width, height);
    mCommandBuffer->SetViewport(0.0f, static_cast<float>(width),
                                0.0f, static_cast<float>(height), 0.0f, 1.0f);
}

void DrawTest::EndTestFrame()
{
    mCommandBuffer->SetRenderTarget(nullptr);

    // copy texture to readback texture
    for (size_t i = 0; i < mTargetTextures.size(); ++i)
    {
        mCommandBuffer->CopyTexture(mTargetTextures[i].texture,
                                    mTargetTextures[i].readbackTexture);
    }

    CommandListID commandList = mCommandBuffer->Finish();
    ASSERT_NE(0u, commandList);

    ASSERT_TRUE(gRendererDevice->Execute(commandList));
    ASSERT_TRUE(gRendererDevice->FinishFrame());
    gRendererDevice->WaitForGPU();

    // download pixel data from readback texture
    for (size_t i = 0; i < mTargetTextures.size(); ++i)
    {
        TargetTexture& target = mTargetTextures[i];
        target.pixelData.reset(new char[target.textureSize]);
        ASSERT_TRUE(gRendererDevice->DownloadTexture(target.readbackTexture, target.pixelData.get()));
    }
}
