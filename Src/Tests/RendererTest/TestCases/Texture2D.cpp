#include "../PCH.hpp"
#include "../RendererTest.hpp"
#include "Engine/Common/Utils/TaskBuilder.hpp"
#include "Engine/Common/Utils/Waitable.hpp"
#include "Engine/Common/Math/Math.hpp"
#include "Engine/Common/Math/Random.hpp"

class Texture2D : public RendererTest
{
};

TEST_F(Texture2D, Creation)
{
    TexturePtr texture;

    const uint32 bitmap[] = { 0xFFFFFFFF, 0, 0, 0xFFFFFFFF };

    TextureDataDesc textureDataDesc;
    textureDataDesc.data = bitmap;
    textureDataDesc.lineSize = 2 * sizeof(uint32);
    textureDataDesc.sliceSize = 4 * sizeof(uint32);

    // default (good) texture descriptor
    TextureDesc defTextureDesc;
    defTextureDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER;
    defTextureDesc.format = Format::R8G8B8A8_U_Norm;
    defTextureDesc.width = 2;
    defTextureDesc.height = 2;
    defTextureDesc.mipmaps = 1;
    defTextureDesc.dataDesc = &textureDataDesc;
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

TEST_F(Texture2D, Download)
{
    const uint32 width = 235;
    const uint32 height = 117;

    Math::Random random;
    Common::DynArray<uint32> initData(width * height);
    for (uint32 i = 0; i < width * height; ++i)
    {
        initData[i] = random.GetInt();
    }

    TextureDataDesc textureDataDesc;
    textureDataDesc.data = initData.Data();
    textureDataDesc.lineSize = width * sizeof(uint32);
    textureDataDesc.sliceSize = width * height * sizeof(uint32);

    TextureDesc texDesc;
    texDesc.format = Format::R8G8B8A8_U_Norm;
    texDesc.width = width;
    texDesc.height = height;
    texDesc.dataDesc = &textureDataDesc;

    const TexturePtr texture = gRendererDevice->CreateTexture(texDesc);
    EXPECT_TRUE(texture.Get() != nullptr) << "Texture was not created despite correct data.";

    Common::DynArray<uint32> readData(width * height);
    const auto readCallback = [&](const void* data, size_t, size_t rowPitch)
    {
        for (uint32 row = 0; row < height; ++row)
        {
            memcpy(
                (uint8*)readData.Data() + row * width * sizeof(uint32),
                (const uint8*)data + row * rowPitch,
                width * sizeof(uint32));
        }
    };

    Common::Waitable waitable;
    {
        Common::TaskBuilder builder{ waitable };
        ASSERT_TRUE(gRendererDevice->DownloadTexture(texture, readCallback, builder));
    }
    waitable.Wait();

    for (uint32 i = 0; i < width * height; ++i)
    {
        EXPECT_EQ(initData[i], readData[i]);
    }
}