#include "PCH.hpp"
#include "RendererTest.hpp"

class ShaderTest : public RendererTest
{
};


// TODO: handle OpenGL shaders
const std::string TEST_SHADER_PATH = "nfEngine/nfRendererTest/Shaders/D3D11/Simple.hlsl";


TEST_F(ShaderTest, GetIODesc)
{
    ShaderDesc desc;
    desc.path = TEST_SHADER_PATH.c_str();
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
    ASSERT_NE(ioDesc.resBinding.end(), cbufferDesc);
    EXPECT_EQ(1, cbufferDesc->second.slot);
    EXPECT_EQ(ShaderResourceType::CBuffer, cbufferDesc->second.type);

    const auto textureDesc = ioDesc.resBinding.find("TestTexture");
    ASSERT_NE(ioDesc.resBinding.end(), textureDesc);
    EXPECT_EQ(3, textureDesc->second.slot);
    EXPECT_EQ(ShaderResourceType::Texture, textureDesc->second.type);

    const auto samplerDesc = ioDesc.resBinding.find("TestSampler");
    ASSERT_NE(ioDesc.resBinding.end(), samplerDesc);
    EXPECT_EQ(5, samplerDesc->second.slot);
    EXPECT_EQ(ShaderResourceType::Sampler, samplerDesc->second.type);
}

TEST_F(ShaderTest, Disassemble)
{
    ShaderDesc desc;
    desc.path = TEST_SHADER_PATH.c_str();
    desc.type = ShaderType::Vertex;

    /// compile shader
    std::unique_ptr<IShader> shader;
    shader.reset(gRendererDevice->CreateShader(desc));
    ASSERT_TRUE(shader != nullptr);

    /// get shader IO description
    ShaderIODesc ioDesc;
    ASSERT_TRUE(shader->GetIODesc(ioDesc));
    EXPECT_EQ(3, ioDesc.resBinding.size());

    std::string disasm;
    ASSERT_TRUE(shader->Disassemble(false, disasm));
    EXPECT_TRUE(disasm.length() > 0);
    std::cout << disasm << std::endl;
}