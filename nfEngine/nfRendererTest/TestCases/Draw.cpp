#include "../PCH.hpp"
#include "../Backends.hpp"
#include "../RendererTest.hpp"

#include "nfCommon/Math/Math.hpp"


using namespace Math;

const int VIEWPORT_SIZE = 16;
const Float4 CLEAR_COLOR = Float4(0.0f, 0.0f, 0.0f, 0.0f);

class SimpleDrawTest : public RendererTest
{
private:
    std::vector<std::unique_ptr<IShader>> mVertexShaders;
    std::vector<std::unique_ptr<IShader>> mPixelShaders;
    std::vector<std::unique_ptr<IShaderProgram>> mShaderPrograms;

protected:
    struct VertexFormat
    {
        Float3 pos;
        Float2 texCoord;
        uint32 color;

        VertexFormat(const Math::Float3& pos, const Math::Float2& texCoord, uint32 color)
            : pos(pos), texCoord(texCoord), color(color)
        {
        }
    };

    std::unique_ptr<IBuffer> mVertexBuffer;
    std::unique_ptr<IBuffer> mIndexBuffer;
    std::unique_ptr<IVertexLayout> mVertexLayout;

    Float4 mPixels[VIEWPORT_SIZE][VIEWPORT_SIZE];

    IShader* CompileShader(const char* path, ShaderType type, ShaderMacro* macros = nullptr,
                           size_t macrosNum = 0)
    {
        const std::string shaderPath = gShaderPathPrefix + path + gShaderPathExt;

        ShaderDesc desc;
        desc.type = type;
        desc.path = shaderPath.c_str();
        desc.macros = macros;
        desc.macrosNum = macrosNum;
        return gRendererDevice->CreateShader(desc);
    }

    void CreateShaderProgram(IShaderProgram** shaderProgram, const char* vsPath, const char* psPath,
                             ShaderMacro* macros = nullptr, size_t macrosNum = 0)
    {
        IShader* vs = CompileShader(vsPath, ShaderType::Vertex, macros, macrosNum);
        ASSERT_NE(nullptr, vs);
        mVertexShaders.emplace_back(std::unique_ptr<IShader>(vs));

        IShader* ps = CompileShader(psPath, ShaderType::Pixel, macros, macrosNum);
        ASSERT_NE(nullptr, ps);
        mPixelShaders.emplace_back(std::unique_ptr<IShader>(ps));

        ShaderProgramDesc shaderProgramDesc;
        shaderProgramDesc.vertexShader = vs;
        shaderProgramDesc.pixelShader = ps;
        *shaderProgram = gRendererDevice->CreateShaderProgram(shaderProgramDesc);
        ASSERT_NE(nullptr, (*shaderProgram));
        mShaderPrograms.emplace_back(std::unique_ptr<IShaderProgram>(*shaderProgram));
    }

    void SetUp() override
    {
        // vertex buffer
        const VertexFormat vbData[] =
        {
            VertexFormat(Float3(-1.0f, -1.0f, 0.0f), Float2(0.0f, 0.0f), 0xFFFFFFFF),
            VertexFormat(Float3(-1.0f,  1.0f, 0.0f), Float2(0.0f, 1.0f), 0xFFFFFFFF),
            VertexFormat(Float3( 1.0f, -1.0f, 0.0f), Float2(1.0f, 0.0f), 0xFFFFFFFF),
            VertexFormat(Float3( 1.0f,  1.0f, 0.0f), Float2(1.0f, 1.0f), 0xFFFFFFFF),
        };

        BufferDesc vbDesc;
        vbDesc.type = BufferType::Vertex;
        vbDesc.access = BufferAccess::GPU_ReadOnly;
        vbDesc.size = sizeof(vbData);
        vbDesc.initialData = vbData;
        mVertexBuffer.reset(gRendererDevice->CreateBuffer(vbDesc));
        ASSERT_NE(nullptr, mVertexBuffer.get());


        // index buffer
        uint16 ibData[] =
        {
            0, 2, 3,    0, 3, 1,
        };

        BufferDesc ibDesc;
        ibDesc.type = BufferType::Index;
        ibDesc.access = BufferAccess::GPU_ReadOnly;
        ibDesc.size = sizeof(ibData);
        ibDesc.initialData = ibData;
        mIndexBuffer.reset(gRendererDevice->CreateBuffer(ibDesc));
        ASSERT_NE(nullptr, mIndexBuffer.get());


        // vertex layout
        VertexLayoutElement vertexLayoutElements[] =
        {
            { ElementFormat::Float_32,      3,  0, 0, false, 0 }, // position
            { ElementFormat::Float_32,      2, 12, 0, false, 0 }, // tex-coords
            { ElementFormat::Uint_8_norm,   4, 20, 0, false, 0 }, // color
        };

        VertexLayoutDesc vertexLayoutDesc;
        vertexLayoutDesc.elements = vertexLayoutElements;
        vertexLayoutDesc.numElements = 3;
        mVertexLayout.reset(gRendererDevice->CreateVertexLayout(vertexLayoutDesc));
        ASSERT_NE(nullptr, mVertexLayout.get());
    }

    void VerifyPixels(const Float4& expected)
    {
        for (int i = 0; i < VIEWPORT_SIZE; ++i)
        {
            for (int j = 0; j < VIEWPORT_SIZE; ++j)
            {
                SCOPED_TRACE("i = " + std::to_string(i) + ", = " + std::to_string(j));
                ASSERT_NEAR(expected.x, mPixels[i][j].x, NFE_MATH_EPSILON);
                ASSERT_NEAR(expected.y, mPixels[i][j].y, NFE_MATH_EPSILON);
                ASSERT_NEAR(expected.z, mPixels[i][j].z, NFE_MATH_EPSILON);
                ASSERT_NEAR(expected.w, mPixels[i][j].w, NFE_MATH_EPSILON);
            }
        }
    }
};

TEST_F(SimpleDrawTest, QuadConstantColor)
{
    std::unique_ptr<IResourceBindingLayout> resBindingLayout;
    std::unique_ptr<IPipelineState> pipelineState;

    IShaderProgram* shaderProgram = nullptr;
    CreateShaderProgram(&shaderProgram, "SimpleDrawVS", "SimpleDrawPS");
    ASSERT_NE(nullptr, shaderProgram);

    resBindingLayout.reset(gRendererDevice->CreateResourceBindingLayout(ResourceBindingLayoutDesc()));
    ASSERT_NE(nullptr, resBindingLayout.get());

    const CullMode cullModes[] =
    {
        CullMode::Disabled,
        CullMode::CW,
        CullMode::CCW,
    };

    for (size_t i = 0; i < ArraySize(cullModes); ++i)
    {
        Float4 expectedColor = CLEAR_COLOR;

        const char* cullModeStr = "unknown";
        switch (cullModes[i])
        {
        case CullMode::Disabled:
            cullModeStr = "Disabled";
            expectedColor = Float4(1.0f, 1.0f, 1.0f, 1.0f);
            break;
        case CullMode::CW:
            cullModeStr = "CW";
            expectedColor = Float4(1.0f, 1.0f, 1.0f, 1.0f);
            break;
        case CullMode::CCW:
            cullModeStr = "CCW";
            break;
        }

        SCOPED_TRACE("CullMode: " + std::string(cullModeStr));

        RasterizerStateDesc rasterizerDesc;
        rasterizerDesc.cullMode = cullModes[i];

        PipelineStateDesc pipelineStateDesc;
        pipelineStateDesc.raterizerState = rasterizerDesc;
        pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
        pipelineStateDesc.vertexLayout = mVertexLayout.get();
        pipelineStateDesc.resBindingLayout = resBindingLayout.get();
        pipelineState.reset(gRendererDevice->CreatePipelineState(pipelineStateDesc));
        ASSERT_NE(nullptr, pipelineState.get());


        BeginTestFrame(VIEWPORT_SIZE, VIEWPORT_SIZE, ElementFormat::Float_32, 4);
        {
            mCommandBuffer->Clear(NFE_CLEAR_FLAG_TARGET, reinterpret_cast<const float*>(&CLEAR_COLOR.x));

            IBuffer* vb = mVertexBuffer.get();
            int stride = sizeof(VertexFormat);
            int offset = 0;
            mCommandBuffer->SetVertexBuffers(1, &vb, &stride, &offset);
            mCommandBuffer->SetIndexBuffer(mIndexBuffer.get(), IndexBufferFormat::Uint16);
            mCommandBuffer->SetResourceBindingLayout(resBindingLayout.get());
            mCommandBuffer->SetPipelineState(pipelineState.get());
            mCommandBuffer->SetShaderProgram(shaderProgram);

            mCommandBuffer->DrawIndexed(6, 1);
        }
        EndTestFrame(mPixels);

        VerifyPixels(expectedColor);
    }
}

TEST_F(SimpleDrawTest, QuadStaticCBufferColor)
{
    const Float4 customColor = Float4(0.123f, -654.0f, 983.0f, 1.0f);

    std::unique_ptr<IBuffer> constatnBuffer;
    std::unique_ptr<IResourceBindingSet> resBindingSet;
    std::unique_ptr<IResourceBindingInstance> resBindingInstance;
    std::unique_ptr<IResourceBindingLayout> resBindingLayout;
    std::unique_ptr<IPipelineState> pipelineState;

    IShaderProgram* shaderProgram = nullptr;
    ShaderMacro macro("USE_CBUFFER", "1");
    CreateShaderProgram(&shaderProgram, "SimpleDrawVS", "SimpleDrawPS", &macro, 1);
    ASSERT_NE(nullptr, shaderProgram);

    BufferDesc cbufferDesc;
    cbufferDesc.type = BufferType::Constant;
    cbufferDesc.access = BufferAccess::GPU_ReadOnly;
    cbufferDesc.size = sizeof(Float4);
    cbufferDesc.initialData = &customColor;
    constatnBuffer.reset(gRendererDevice->CreateBuffer(cbufferDesc));
    ASSERT_NE(nullptr, constatnBuffer.get());

    // create resource binding set for cbuffer
    int cbufferSlot = shaderProgram->GetResourceSlotByName("TestCBuffer");
    ASSERT_NE(-1, cbufferSlot);
    ResourceBindingDesc binding(ShaderResourceType::CBuffer, cbufferSlot);
    resBindingSet.reset(gRendererDevice->CreateResourceBindingSet(
        ResourceBindingSetDesc(&binding, 1, ShaderType::Vertex)));
    ASSERT_NE(nullptr, resBindingSet.get());

    resBindingInstance.reset(gRendererDevice->CreateResourceBindingInstance(resBindingSet.get()));
    ASSERT_NE(nullptr, resBindingInstance.get());
    ASSERT_TRUE(resBindingInstance->WriteCBufferView(0, constatnBuffer.get()));

    IResourceBindingSet* sets[] = { resBindingSet.get() };
    resBindingLayout.reset(gRendererDevice->CreateResourceBindingLayout(
        ResourceBindingLayoutDesc(sets, 1)));
    ASSERT_NE(nullptr, resBindingLayout.get());

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
    pipelineStateDesc.vertexLayout = mVertexLayout.get();
    pipelineStateDesc.resBindingLayout = resBindingLayout.get();
    pipelineState.reset(gRendererDevice->CreatePipelineState(pipelineStateDesc));
    ASSERT_NE(nullptr, pipelineState.get());


    BeginTestFrame(VIEWPORT_SIZE, VIEWPORT_SIZE, ElementFormat::Float_32, 4);
    {
        mCommandBuffer->Clear(NFE_CLEAR_FLAG_TARGET, reinterpret_cast<const float*>(&CLEAR_COLOR.x));

        IBuffer* vb = mVertexBuffer.get();
        int stride = sizeof(VertexFormat);
        int offset = 0;
        mCommandBuffer->SetVertexBuffers(1, &vb, &stride, &offset);
        mCommandBuffer->SetIndexBuffer(mIndexBuffer.get(), IndexBufferFormat::Uint16);
        mCommandBuffer->SetResourceBindingLayout(resBindingLayout.get());
        mCommandBuffer->SetPipelineState(pipelineState.get());
        mCommandBuffer->SetShaderProgram(shaderProgram);
        mCommandBuffer->BindResources(0, resBindingInstance.get());

        mCommandBuffer->DrawIndexed(6, 1);
    }
    EndTestFrame(mPixels);

    VerifyPixels(customColor);
}