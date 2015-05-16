#include "PCH.hpp"
#include "RendererTest.hpp"

class ShaderTest : public RendererTest
{
};

TEST_F(ShaderTest, GetIODesc)
{
    ShaderDesc desc;
    desc.path = "nfEngine/nfRendererTest/Shaders/D3D11/Simple.hlsl";
    desc.type = ShaderType::Vertex;

    /// compile shader
    std::unique_ptr<IShader> shader;
    shader.reset(gRendererDevice->CreateShader(desc));
    ASSERT_TRUE(shader != nullptr);

    /// get shader IO description
    ShaderIODesc ioDesc;
    ASSERT_TRUE(shader->GetIODesc(ioDesc));
    EXPECT_EQ(3, ioDesc.resBinding.size());

    /// verify resources bindings
    const auto cbufferDesc = ioDesc.resBinding.find("TestCBuffer");
    EXPECT_NE(ioDesc.resBinding.end(), cbufferDesc);
    if (cbufferDesc != ioDesc.resBinding.end())
    {
        EXPECT_EQ(1, cbufferDesc->second.slot);
        EXPECT_EQ(ShaderResourceType::CBuffer, cbufferDesc->second.type);
    }

    const auto textureDesc = ioDesc.resBinding.find("TestTexture");
    EXPECT_NE(ioDesc.resBinding.end(), textureDesc);
    if (textureDesc != ioDesc.resBinding.end())
    {
        EXPECT_EQ(3, textureDesc->second.slot);
        EXPECT_EQ(ShaderResourceType::Texture, textureDesc->second.type);
    }

    const auto samplerDesc = ioDesc.resBinding.find("TestSampler");
    EXPECT_NE(ioDesc.resBinding.end(), samplerDesc);
    if (samplerDesc != ioDesc.resBinding.end())
    {
        EXPECT_EQ(5, samplerDesc->second.slot);
        EXPECT_EQ(ShaderResourceType::Sampler, samplerDesc->second.type);
    }
}
