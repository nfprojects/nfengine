#include "../PCH.hpp"
#include "../Backends.hpp"
#include "../RendererTest.hpp"

using namespace NFE::Common;

class ShaderTest : public RendererTest
{
};

TEST_F(ShaderTest, Disassemble)
{
    const String shaderPath = gShaderPathPrefix + "Simple" + gShaderPathExt;

    ShaderDesc desc;
    desc.path = shaderPath.Str();
    desc.type = ShaderType::Vertex;

    /// compile shader
    ShaderPtr shader;
    shader = gRendererDevice->CreateShader(desc);
    ASSERT_TRUE(shader != nullptr);

    String disasm;
    ASSERT_TRUE(shader->Disassemble(false, disasm));
    EXPECT_TRUE(disasm.Length() > 0);
}
