#include "PCH.hpp"
#include "Backends.hpp"
#include "DrawTest.hpp"
#include "Engine/Common/Utils/TaskBuilder.hpp"
#include "Engine/Common/Utils/Waitable.hpp"
#include "Engine/Renderers/RendererCommon/Fence.hpp"


void DrawTest::BeginTestFrame(uint32 width, uint32 height, uint32 numTargets, const Format* formats)
{
    mTestTextureWidth = width;
    mTestTextureHeight = height;

    mTargetTextures.Clear();

    mCommandBuffer = gRendererDevice->CreateCommandRecorder();
    ASSERT_FALSE(!mCommandBuffer);

    for (uint32 i = 0; i < numTargets; ++i)
    {
        TargetTexture targetTexture;
        targetTexture.format = formats[i];
        targetTexture.textureRowPitch = width * GetElementFormatSize(formats[i]);
        targetTexture.textureSize = width * targetTexture.textureRowPitch;

        TextureDesc texDesc;
        texDesc.binding = NFE_RENDERER_TEXTURE_BIND_RENDERTARGET;
        texDesc.mode = ResourceAccessMode::GPUOnly;
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

        mTargetTextures.EmplaceBack(std::move(targetTexture));
    }

    ASSERT_TRUE(mCommandBuffer->Begin(CommandQueueType::Graphics));
    mCommandBuffer->SetRenderTarget(mTestRenderTarget);
    mCommandBuffer->SetScissors(0, 0, width, height);
    mCommandBuffer->SetViewport(0.0f, static_cast<float>(width),
                                0.0f, static_cast<float>(height), 0.0f, 1.0f);
}

void DrawTest::EndTestFrame()
{
    CommandListPtr commandList = mCommandBuffer->Finish();
    ASSERT_TRUE(commandList != nullptr);

    gMainCommandQueue->Execute(commandList);
    ASSERT_TRUE(gRendererDevice->FinishFrame());

    Common::Waitable waitable;
    {
        Common::TaskBuilder builder{ waitable };

        for (TargetTexture& targetTexture : mTargetTextures)
        {
            targetTexture.pixelData = Common::MakeUniquePtr<uint8[]>(targetTexture.textureSize);

            const auto readCallback = [&](const void* data, size_t totalSize, size_t rowPitch)
            {
                NFE_ASSERT(totalSize >= targetTexture.textureSize, "Invalid read size");
                NFE_ASSERT(rowPitch >= targetTexture.textureRowPitch, "Invalid row pitch");

                for (uint32 row = 0; row < mTestTextureHeight; ++row)
                {
                    memcpy(
                        targetTexture.pixelData.Get() + row * mTestTextureWidth * sizeof(uint32),
                        (const uint8*)data + row * rowPitch,
                        mTestTextureWidth * sizeof(uint32));
                }
            };

            ASSERT_TRUE(gRendererDevice->DownloadTexture(targetTexture.texture, readCallback, builder));
        }
    }
    waitable.Wait();
}
