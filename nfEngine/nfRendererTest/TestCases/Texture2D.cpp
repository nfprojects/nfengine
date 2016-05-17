#include "../PCH.hpp"
#include "../RendererTest.hpp"
#include "../../nfCommon/Math/Math.hpp"

class Texture2D : public RendererTest
{
};

TEST_F(Texture2D, Clear)
{
    const float color[] = { 0.1f, 0.2f, 0.3f, 0.4f };
    float pixels[16][16][4];

    BeginTestFrame(16, 16, ElementFormat::Float_32, 4);

    mCommandBuffer->SetViewport(0.0f, 16.0f, 0.0f, 16.0f, 0.0f, 1.0f);
    mCommandBuffer->Clear(NFE_CLEAR_FLAG_TARGET, color);

    EndTestFrame(pixels);

    for (int i = 0; i < 16; ++i)
        for (int j = 0; j < 16; ++j)
        {
            SCOPED_TRACE("i = " + std::to_string(i) + ", = " + std::to_string(j));
            EXPECT_NEAR(color[0], pixels[i][j][0], NFE_MATH_EPSILON);
            EXPECT_NEAR(color[1], pixels[i][j][1], NFE_MATH_EPSILON);
            EXPECT_NEAR(color[2], pixels[i][j][2], NFE_MATH_EPSILON);
            EXPECT_NEAR(color[3], pixels[i][j][3], NFE_MATH_EPSILON);
        }
}

TEST_F(Texture2D, Creation)
{
    std::unique_ptr<ITexture> texture;

    uint32_t bitmap[] = { 0xFFFFFFFF, 0, 0, 0xFFFFFFFF };

    TextureDataDesc textureDataDesc;
    textureDataDesc.data = bitmap;
    textureDataDesc.lineSize = 2 * sizeof(uint32_t);
    textureDataDesc.sliceSize = 4 * sizeof(uint32_t);

    // default (good) texture descriptor
    TextureDesc defTextureDesc;
    defTextureDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER;
    defTextureDesc.format = ElementFormat::Uint_8_norm;
    defTextureDesc.texelSize = 4;
    defTextureDesc.width = 2;
    defTextureDesc.height = 2;
    defTextureDesc.mipmaps = 1;
    defTextureDesc.dataDesc = &textureDataDesc;
    defTextureDesc.layers = 1;

    TextureDesc textureDesc;

    // no texture data suplied
    textureDesc = defTextureDesc;
    textureDesc.dataDesc = nullptr;
    texture.reset(gRendererDevice->CreateTexture(textureDesc));
    EXPECT_TRUE(texture.get() == nullptr) << "Texture was created despite null data desc.";

    // invalid width
    textureDesc = defTextureDesc;
    textureDesc.width = 0;
    texture.reset(gRendererDevice->CreateTexture(textureDesc));
    EXPECT_TRUE(texture.get() == nullptr) << "Texture was created despite incorrect width.";

    // invalid height
    textureDesc = defTextureDesc;
    textureDesc.height = 0;
    texture.reset(gRendererDevice->CreateTexture(textureDesc));
    EXPECT_TRUE(texture.get() == nullptr) << "Texture was created despite incorrect height.";

    // TODO: write more cases

    textureDesc = defTextureDesc;
    texture.reset(gRendererDevice->CreateTexture(textureDesc));
    EXPECT_TRUE(texture.get() != nullptr) << "Texture was not created despite correct data.";
}
