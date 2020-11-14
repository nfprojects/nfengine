#include "../PCH.hpp"
#include "../RendererTest.hpp"
#include "Engine/Common/Utils/TaskBuilder.hpp"
#include "Engine/Common/Utils/Waitable.hpp"
#include "Engine/Common/Math/Math.hpp"
#include "Engine/Common/Math/Random.hpp"
#include "Engine/Renderers/RendererCommon/Fence.hpp"

class Texture2D : public RendererTest
{
};

TEST_F(Texture2D, Creation)
{
    TexturePtr texture;

    // default (good) texture descriptor
    TextureDesc defTextureDesc;
    defTextureDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER;
    defTextureDesc.format = Format::R8G8B8A8_U_Norm;
    defTextureDesc.width = 2;
    defTextureDesc.height = 2;
    defTextureDesc.mipmaps = 1;
    defTextureDesc.layers = 1;

    TextureDesc textureDesc;

    // invalid width
    textureDesc = defTextureDesc;
    textureDesc.width = 0;
    texture = gRendererDevice->CreateTexture(textureDesc);
    EXPECT_TRUE(texture.Get() == nullptr) << "Texture was created despite incorrect width.";

    // invalid height
    textureDesc = defTextureDesc;
    textureDesc.height = 0;
    texture = gRendererDevice->CreateTexture(textureDesc);
    EXPECT_TRUE(texture.Get() == nullptr) << "Texture was created despite incorrect height.";

    // TODO: write more cases

    textureDesc = defTextureDesc;
    texture = gRendererDevice->CreateTexture(textureDesc);
    EXPECT_TRUE(texture.Get() != nullptr) << "Texture was not created despite correct data.";
}

TEST_F(Texture2D, UploadAndDownload)
{
    // overall texture size
    const uint32 textureWidth = 235;
    const uint32 textureHeight = 117;

    // copy source region
    const uint32 copyWidth = 74;
    const uint32 copyHeight = 56;
    const uint32 copyLeft = 121;
    const uint32 copyTop = 17;

    TexturePlacementInfo texturePlacementInfo;
    ASSERT_TRUE(gRendererDevice->CalculateTexturePlacementInfo(Format::R8G8B8A8_U_Norm, copyWidth, copyHeight, 1, texturePlacementInfo));
    ASSERT_GT(texturePlacementInfo.alignment, 0u);
    ASSERT_GT(texturePlacementInfo.rowPitch, 0u);
    ASSERT_GT(texturePlacementInfo.totalSize, 0u);
    ASSERT_TRUE(texturePlacementInfo.rowPitch % sizeof(uint32) == 0);

    const uint32 stride = (uint32)texturePlacementInfo.rowPitch / sizeof(uint32);
    const uint32 bufferSize = stride * copyHeight;

    TextureDesc texDesc;
    texDesc.format = Format::R8G8B8A8_U_Norm;
    texDesc.width = textureWidth;
    texDesc.height = textureHeight;

    const TexturePtr texture = gRendererDevice->CreateTexture(texDesc);
    EXPECT_TRUE(texture.Get() != nullptr);

    BufferDesc bufferDesc;
    bufferDesc.mode = ResourceAccessMode::Readback;
    bufferDesc.size = bufferSize * sizeof(uint32);

    const BufferPtr readbackBuffer = gRendererDevice->CreateBuffer(bufferDesc);
    EXPECT_TRUE(readbackBuffer.Get() != nullptr);

    CommandRecorderPtr recorder = gRendererDevice->CreateCommandRecorder();

    Math::Random random;
    Common::DynArray<uint32> initData(textureWidth * textureHeight);
    for (uint32 i = 0; i < textureWidth * textureHeight; ++i)
    {
        initData[i] = random.GetInt();
    }

    // upload texture data
    FencePtr fenceAfterUpload;
    {
        recorder->Begin(CommandQueueType::Copy);
        recorder->WriteTexture(texture, initData.Data());
        gCopyCommandQueue->Execute(recorder->Finish());
        fenceAfterUpload = gCopyCommandQueue->Signal();
    }

    // copy texture to buffer
    FencePtr fenceAfterReadback;
    {
        TextureRegion copyRegion;
        copyRegion.width = copyWidth;
        copyRegion.height = copyHeight;
        copyRegion.x = copyLeft;
        copyRegion.y = copyTop;

        recorder->Begin(CommandQueueType::Copy);
        recorder->CopyTextureToBuffer(texture, readbackBuffer, &copyRegion);
        gCopyCommandQueue->Execute(recorder->Finish(), fenceAfterUpload);
        fenceAfterReadback = gCopyCommandQueue->Signal();
    }

    recorder.Reset();

    fenceAfterReadback->Wait();

    Common::DynArray<uint32> readData;
    readData.Resize(bufferSize);
    {
        void* mappedMemory = readbackBuffer->Map();
        ASSERT_NE(nullptr, mappedMemory);
        memcpy(readData.Data(), mappedMemory, bufferSize * sizeof(uint32));
        readbackBuffer->Unmap();
    }

    for (uint32 y = 0; y < copyHeight; ++y)
    {
        for (uint32 x = 0; x < copyWidth; ++x)
        {
            const uint32 imageOffset = textureWidth * (y + copyTop) + x + copyLeft;
            const uint32 bufferOffset = stride * y + x;
            EXPECT_EQ(initData[imageOffset], readData[bufferOffset]) << "x=" << x << ",y=" << y;
        }
    }
}
