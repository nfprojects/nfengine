#include "../PCH.hpp"
#include "../RendererTest.hpp"
#include "nfCommon/Math/Math.hpp"

class Texture2D : public RendererTest
{
};

TEST_F(Texture2D, Clear)
{
    const Math::Float4 color(0.1f, 0.2f, 0.3f, 0.4f);
    float pixels[16][16][4];

    BeginTestFrame(16, 16, ElementFormat::R32G32B32A32_Float);

    mCommandBuffer->SetViewport(0.0f, 16.0f, 0.0f, 16.0f, 0.0f, 1.0f);
    mCommandBuffer->Clear(ClearFlagsColor, 1, nullptr, &color);

    EndTestFrame(pixels);

    for (int i = 0; i < 16; ++i)
        for (int j = 0; j < 16; ++j)
        {
            SCOPED_TRACE("i = " + std::to_string(i) + ", = " + std::to_string(j));
            EXPECT_NEAR(color.x, pixels[i][j][0], NFE_MATH_EPSILON);
            EXPECT_NEAR(color.y, pixels[i][j][1], NFE_MATH_EPSILON);
            EXPECT_NEAR(color.z, pixels[i][j][2], NFE_MATH_EPSILON);
            EXPECT_NEAR(color.w, pixels[i][j][3], NFE_MATH_EPSILON);
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
    defTextureDesc.format = ElementFormat::R8G8B8A8_U_Norm;
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
