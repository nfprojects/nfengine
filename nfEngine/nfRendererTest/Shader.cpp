#include "PCH.hpp"
#include "RendererTest.hpp"

class ShaderTest : public RendererTest
{
};

// TODO: handle OpenGL shaders
const std::string TEST_SHADER_PATH = "nfEngine/nfRendererTest/Shaders/D3D11/Simple.hlsl";

TEST_F(ShaderTest, Disassemble)
{
    const size_t resourceBindings = 3;

    ShaderDesc desc;
    desc.path = TEST_SHADER_PATH.c_str();
    desc.type = ShaderType::Vertex;

    /// compile shader
    std::unique_ptr<IShader> shader;
    shader.reset(gRendererDevice->CreateShader(desc));
    ASSERT_TRUE(shader != nullptr);

    std::string disasm;
    ASSERT_TRUE(shader->Disassemble(false, disasm));
    EXPECT_TRUE(disasm.length() > 0);
}