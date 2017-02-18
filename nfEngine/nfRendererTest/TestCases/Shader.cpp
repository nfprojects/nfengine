#include "../PCH.hpp"
#include "../Backends.hpp"
#include "../RendererTest.hpp"

class ShaderTest : public RendererTest
{
};

TEST_F(ShaderTest, Disassemble)
{
    const size_t resourceBindings = 3;

    const std::string shaderPath = gShaderPathPrefix + "Simple" + gShaderPathExt;

    ShaderDesc desc;
    desc.path = shaderPath.c_str();
    desc.type = ShaderType::Vertex;

    /// compile shader
    ShaderPtr shader;
    shader.reset(gRendererDevice->CreateShader(desc));
    ASSERT_TRUE(shader != nullptr);

    std::string disasm;
    ASSERT_TRUE(shader->Disassemble(false, disasm));
    EXPECT_TRUE(disasm.length() > 0);
}
