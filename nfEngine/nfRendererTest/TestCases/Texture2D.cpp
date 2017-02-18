#include "../PCH.hpp"
#include "../RendererTest.hpp"
#include "nfCommon/Math/Math.hpp"

class Texture2D : public RendererTest
{
};

TEST_F(Texture2D, Creation)
{
    TexturePtr texture;

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

    // no texture data supplied
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
