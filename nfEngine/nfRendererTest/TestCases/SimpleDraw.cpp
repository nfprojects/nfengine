#include "../PCH.hpp"
#include "../Backends.hpp"
#include "../DrawTest.hpp"

#include "nfCommon/Math/Math.hpp"
#include "nfCommon/Math/Conversions.hpp"
#include "nfCommon/Math/Float4.hpp"

using namespace NFE::Common;
using namespace Math;

const int VIEWPORT_SIZE = 16;
const Float4 CLEAR_COLOR = Float4(0.0f, 0.0f, 0.0f, 0.0f);

class SimpleDrawTest : public DrawTest
{
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

    BufferPtr mVertexBuffer;
    BufferPtr mIndexBuffer;
    VertexLayoutPtr mVertexLayout;
    ShaderPtr mVertexShader;
    ShaderPtr mPixelShader;


    ShaderPtr CompileShader(const char* path, ShaderType type, ShaderMacro* macros = nullptr, uint32 macrosNum = 0)
    {
        const String shaderPath = gShaderPathPrefix + path + gShaderPathExt;

        ShaderDesc desc;
        desc.type = type;
        desc.path = shaderPath.Str();
        desc.macros = macros;
        desc.macrosNum = macrosNum;
        return gRendererDevice->CreateShader(desc);
    }

    void CreateShaderProgram(const char* vsPath, const char* psPath, ShaderMacro* macros = nullptr, uint32 macrosNum = 0)
    {
        mVertexShader = CompileShader(vsPath, ShaderType::Vertex, macros, macrosNum);
        ASSERT_NE(nullptr, mVertexShader.Get());

        mPixelShader = CompileShader(psPath, ShaderType::Pixel, macros, macrosNum);
        ASSERT_NE(nullptr, mPixelShader.Get());
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
        vbDesc.mode = BufferMode::Static;
        vbDesc.size = sizeof(vbData);
        vbDesc.initialData = vbData;
        mVertexBuffer = gRendererDevice->CreateBuffer(vbDesc);
        ASSERT_NE(nullptr, mVertexBuffer.Get());

        // index buffer
        uint16 ibData[] =
        {
            0, 2, 3,    0, 3, 1,
        };

        BufferDesc ibDesc;
        ibDesc.type = BufferType::Index;
        ibDesc.mode = BufferMode::Static;
        ibDesc.size = sizeof(ibData);
        ibDesc.initialData = ibData;
        mIndexBuffer = gRendererDevice->CreateBuffer(ibDesc);
        ASSERT_NE(nullptr, mIndexBuffer.Get());


        // vertex layout
        VertexLayoutElement vertexLayoutElements[] =
        {
            { ElementFormat::R32G32B32_Float,   0,  0, false, 0 }, // position
            { ElementFormat::R32G32_Float,      12, 0, false, 0 }, // tex-coords
            { ElementFormat::R8G8B8A8_U_Norm,   20, 0, false, 0 }, // color
        };

        VertexLayoutDesc vertexLayoutDesc;
        vertexLayoutDesc.elements = vertexLayoutElements;
        vertexLayoutDesc.numElements = 3;
        mVertexLayout = gRendererDevice->CreateVertexLayout(vertexLayoutDesc);
        ASSERT_NE(nullptr, mVertexLayout.Get());
    }
};


// draw constant color quad with different polygon culling modes
TEST_F(SimpleDrawTest, Culling)
{
    ResourceBindingLayoutPtr resBindingLayout;
    PipelineStatePtr pipelineState;

    CreateShaderProgram("SimpleDrawVS", "SimpleDrawPS");

    resBindingLayout = gRendererDevice->CreateResourceBindingLayout(ResourceBindingLayoutDesc());
    ASSERT_NE(nullptr, resBindingLayout.Get());

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
        pipelineStateDesc.vertexShader = mVertexShader;
        pipelineStateDesc.pixelShader = mPixelShader;
        pipelineStateDesc.raterizerState = rasterizerDesc;
        pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
        pipelineStateDesc.vertexLayout = mVertexLayout;
        pipelineStateDesc.resBindingLayout = resBindingLayout;
        pipelineStateDesc.numRenderTargets = 1;
        pipelineStateDesc.rtFormats[0] = ElementFormat::R32G32B32A32_Float;
        pipelineState = gRendererDevice->CreatePipelineState(pipelineStateDesc);
        ASSERT_NE(nullptr, pipelineState.Get());

        ElementFormat formats[] = { ElementFormat::R32G32B32A32_Float };
        BeginTestFrame(VIEWPORT_SIZE, VIEWPORT_SIZE, 1, formats);
        {
            mCommandBuffer->Clear(ClearFlagsColor, 1, nullptr, &CLEAR_COLOR);

            const BufferPtr& vb = mVertexBuffer;
            uint32 stride = sizeof(VertexFormat);
            uint32 offset = 0;
            mCommandBuffer->SetVertexBuffers(1, &vb, &stride, &offset);
            mCommandBuffer->SetIndexBuffer(mIndexBuffer, IndexBufferFormat::Uint16);
            mCommandBuffer->SetResourceBindingLayout(resBindingLayout);
            mCommandBuffer->SetPipelineState(pipelineState);

            mCommandBuffer->DrawIndexed(6, 1);
        }
        EndTestFrame();

        VerifyPixelsFloat(0, &expectedColor.x);
    }
}


// clear render targets with various formats
TEST_F(SimpleDrawTest, RenderTargetFormats)
{
    const Float4 testColors[] =
    {
        // TODO: 1.0e+10, -1.0e+20, etc. values should be also tested, by the test fails
        // then with integer formats on nvidia and Intel cards due to driver bug

        Float4(1.0e9f, -1.0e9f, -1.0e-10f, 1.0e-10f),
        Float4(-718424.0f, -8242.246f, -314.246f, -104.825f),
        Float4(-0.245f, -1.0f, -0.003f, -0.422525f),
        Float4(0.002f, 0.874f, 0.231f, 1.0f),
        Float4(1.425f, 834.0f, 122456.0f, 8492875.0f),
    };

    for (size_t colorIndex = 0; colorIndex < ArraySize(testColors); ++colorIndex)
    {
        const Float4 clearColor = testColors[colorIndex];
        SCOPED_TRACE("TestColor = [" +
                     std::to_string(clearColor.x) + ", " +
                     std::to_string(clearColor.y) + ", " +
                     std::to_string(clearColor.z) + ", " +
                     std::to_string(clearColor.w) + ", ");

        for (int i = 0; i < static_cast<int>(ElementFormat::Max); ++i)
        {
            ElementFormat format = static_cast<ElementFormat>(i);
            bool supportedFormat = false;

            switch (format)
            {
            case ElementFormat::R32G32B32A32_Float:
            case ElementFormat::R32G32B32A32_U_Int:
            case ElementFormat::R32G32B32A32_S_Int:
            case ElementFormat::R32G32_Float:
            case ElementFormat::R32G32_U_Int:
            case ElementFormat::R32G32_S_Int:
            case ElementFormat::R32_Float:
            case ElementFormat::R32_U_Int:
            case ElementFormat::R32_S_Int:
            case ElementFormat::R16G16B16A16_Float:
            case ElementFormat::R16G16B16A16_S_Int:
            case ElementFormat::R16G16B16A16_S_Norm:
            case ElementFormat::R16G16B16A16_U_Int:
            case ElementFormat::R16G16B16A16_U_Norm:
            case ElementFormat::R16G16_Float:
            case ElementFormat::R16G16_S_Int:
            case ElementFormat::R16G16_S_Norm:
            case ElementFormat::R16G16_U_Int:
            case ElementFormat::R16G16_U_Norm:
            case ElementFormat::R16_Float:
            case ElementFormat::R16_S_Int:
            case ElementFormat::R16_S_Norm:
            case ElementFormat::R16_U_Int:
            case ElementFormat::R16_U_Norm:
            case ElementFormat::R8G8B8A8_S_Int:
            case ElementFormat::R8G8B8A8_U_Int:
            case ElementFormat::R8G8B8A8_S_Norm:
            case ElementFormat::R8G8B8A8_U_Norm:
            case ElementFormat::B8G8R8A8_U_Norm:
            case ElementFormat::R8G8_S_Int:
            case ElementFormat::R8G8_U_Int:
            case ElementFormat::R8G8_S_Norm:
            case ElementFormat::R8G8_U_Norm:
            case ElementFormat::R8_S_Int:
            case ElementFormat::R8_U_Int:
            case ElementFormat::R8_S_Norm:
            case ElementFormat::R8_U_Norm:
                supportedFormat = true;
                break;
            }

            if (!supportedFormat)
                continue;

            const char* formatName = GetElementFormatName(format);
            SCOPED_TRACE("Format: " + std::string(formatName));

            BeginTestFrame(VIEWPORT_SIZE, VIEWPORT_SIZE, 1, &format);
            {
                mCommandBuffer->Clear(ClearFlagsColor, 1, nullptr, &clearColor);
            }
            EndTestFrame();

            switch (format)
            {
            // 8-bit unsigned ints
            case ElementFormat::R8_U_Int:
            case ElementFormat::R8G8_U_Int:
            case ElementFormat::R8G8B8A8_U_Int:
            {
                uint8 expected[4];
                expected[0] = ToUint8(clearColor.x);
                expected[1] = ToUint8(clearColor.y);
                expected[2] = ToUint8(clearColor.z);
                expected[3] = ToUint8(clearColor.w);
                VerifyPixelsInteger<uint8>(0, expected);
                break;
            }

            // 8-bit unsigned ints (normalized)
            case ElementFormat::R8_U_Norm:
            case ElementFormat::R8G8_U_Norm:
            case ElementFormat::R8G8B8A8_U_Norm:
            {
                uint8 expected[4];
                expected[0] = ToNormUint8(clearColor.x);
                expected[1] = ToNormUint8(clearColor.y);
                expected[2] = ToNormUint8(clearColor.z);
                expected[3] = ToNormUint8(clearColor.w);
                VerifyPixelsInteger<uint8>(0, expected);
                break;
            }

            // 8-bit signed ints
            //case ElementFormat::R8_S_Int:
            case ElementFormat::R8G8_S_Int:
            case ElementFormat::R8G8B8A8_S_Int:
            {
                int8 expected[4];
                expected[0] = ToInt8(clearColor.x);
                expected[1] = ToInt8(clearColor.y);
                expected[2] = ToInt8(clearColor.z);
                expected[3] = ToInt8(clearColor.w);
                VerifyPixelsInteger<int8>(0, expected);
                break;
            }

            // 8-bit signed ints (normalized)
            case ElementFormat::R8_S_Norm:
            case ElementFormat::R8G8_S_Norm:
            case ElementFormat::R8G8B8A8_S_Norm:
            {
                int8 expected[4];
                expected[0] = ToNormInt8(clearColor.x);
                expected[1] = ToNormInt8(clearColor.y);
                expected[2] = ToNormInt8(clearColor.z);
                expected[3] = ToNormInt8(clearColor.w);
                VerifyPixelsInteger<int8>(0, expected);
                break;
            }

            // 16-bit unsigned ints
            case ElementFormat::R16_U_Int:
            case ElementFormat::R16G16_U_Int:
            case ElementFormat::R16G16B16A16_U_Int:
            {
                uint16 expected[4];
                expected[0] = ToUint16(clearColor.x);
                expected[1] = ToUint16(clearColor.y);
                expected[2] = ToUint16(clearColor.z);
                expected[3] = ToUint16(clearColor.w);
                VerifyPixelsInteger<uint16>(0, expected);
                break;
            }

            // 16-bit unsigned ints (normalized)
            case ElementFormat::R16_U_Norm:
            case ElementFormat::R16G16_U_Norm:
            case ElementFormat::R16G16B16A16_U_Norm:
            {
                uint16 expected[4];
                expected[0] = ToNormUint16(clearColor.x);
                expected[1] = ToNormUint16(clearColor.y);
                expected[2] = ToNormUint16(clearColor.z);
                expected[3] = ToNormUint16(clearColor.w);
                VerifyPixelsInteger<uint16>(0, expected);
                break;
            }

            // 16-bit signed ints
            case ElementFormat::R16_S_Int:
            case ElementFormat::R16G16_S_Int:
            case ElementFormat::R16G16B16A16_S_Int:
            {
                int16 expected[4];
                expected[0] = ToInt16(clearColor.x);
                expected[1] = ToInt16(clearColor.y);
                expected[2] = ToInt16(clearColor.z);
                expected[3] = ToInt16(clearColor.w);
                VerifyPixelsInteger<int16>(0, expected);
                break;
            }

            // 16-bit signed ints (normalized)
            case ElementFormat::R16_S_Norm:
            case ElementFormat::R16G16_S_Norm:
            case ElementFormat::R16G16B16A16_S_Norm:
            {
                int16 expected[4];
                expected[0] = ToNormInt16(clearColor.x);
                expected[1] = ToNormInt16(clearColor.y);
                expected[2] = ToNormInt16(clearColor.z);
                expected[3] = ToNormInt16(clearColor.w);
                VerifyPixelsInteger<int16>(0, expected);
                break;
            }

                // 32-bit unsigned ints
            case ElementFormat::R32_U_Int:
            case ElementFormat::R32G32_U_Int:
            case ElementFormat::R32G32B32_U_Int:
            case ElementFormat::R32G32B32A32_U_Int:
            {
                uint32 expected[4];
                expected[0] = ToUint32(clearColor.x);
                expected[1] = ToUint32(clearColor.y);
                expected[2] = ToUint32(clearColor.z);
                expected[3] = ToUint32(clearColor.w);
                VerifyPixelsInteger<uint32>(0, expected);
                break;
            }

                // 32-bit signed ints
            case ElementFormat::R32_S_Int:
            case ElementFormat::R32G32_S_Int:
            case ElementFormat::R32G32B32_S_Int:
            case ElementFormat::R32G32B32A32_S_Int:
            {
                int32 expected[4];
                expected[0] = ToInt32(clearColor.x);
                expected[1] = ToInt32(clearColor.y);
                expected[2] = ToInt32(clearColor.z);
                expected[3] = ToInt32(clearColor.w);
                VerifyPixelsInteger<int32>(0, expected);
                break;
            }

                // half floats
            case ElementFormat::R16_Float:
            case ElementFormat::R16G16_Float:
            case ElementFormat::R16G16B16A16_Float:
            {
                HalfFloat expected[4];
                expected[0] = ToHalfFloat(clearColor.x);
                expected[1] = ToHalfFloat(clearColor.y);
                expected[2] = ToHalfFloat(clearColor.z);
                expected[3] = ToHalfFloat(clearColor.w);
                VerifyPixelsInteger<HalfFloat>(0, expected);
                break;
            }

                // floats
            case ElementFormat::R32_Float:
            case ElementFormat::R32G32_Float:
            case ElementFormat::R32G32B32_Float:
            case ElementFormat::R32G32B32A32_Float:
                VerifyPixelsFloat(0, &clearColor.x);
                break;

            // packed
            case ElementFormat::B5G6R5_U_norm:
            case ElementFormat::B5G5R5A1_U_Norm:
            case ElementFormat::B4G4R4A4_U_Norm:
            case ElementFormat::R11G11B10_Float:
            case ElementFormat::R10G10B10A2_U_Norm:
            case ElementFormat::R10G10B10A2_U_Int:
            case ElementFormat::R9G9B9E5_Float:
                // TODO: special conversion functions will be required to verify values
                break;
            }
        }
    }
}

TEST_F(SimpleDrawTest, StaticCBuffer)
{
    const Float4 customColor = Float4(0.123f, -654.0f, 983.0f, 1.0f);

    BufferPtr constatnBuffer;
    ResourceBindingSetPtr resBindingSet;
    ResourceBindingInstancePtr resBindingInstance;
    ResourceBindingLayoutPtr resBindingLayout;
    PipelineStatePtr pipelineState;

    ShaderMacro macro("USE_CBUFFER", "1");
    CreateShaderProgram("SimpleDrawVS", "SimpleDrawPS", &macro, 1);

    BufferDesc cbufferDesc;
    cbufferDesc.type = BufferType::Constant;
    cbufferDesc.mode = BufferMode::Static;
    cbufferDesc.size = sizeof(Float4);
    cbufferDesc.initialData = &customColor;
    constatnBuffer = gRendererDevice->CreateBuffer(cbufferDesc);
    ASSERT_NE(nullptr, constatnBuffer.Get());

    // create resource binding set for cbuffer
    int cbufferSlot = mVertexShader->GetResourceSlotByName("TestCBuffer");
    ASSERT_NE(-1, cbufferSlot);
    ResourceBindingDesc binding(ShaderResourceType::CBuffer, cbufferSlot);
    resBindingSet = gRendererDevice->CreateResourceBindingSet(ResourceBindingSetDesc(&binding, 1, ShaderType::Vertex));
    ASSERT_NE(nullptr, resBindingSet.Get());

    resBindingInstance = gRendererDevice->CreateResourceBindingInstance(resBindingSet);
    ASSERT_NE(nullptr, resBindingInstance.Get());
    ASSERT_TRUE(resBindingInstance->WriteCBufferView(0, constatnBuffer));

    ResourceBindingSetPtr sets[] = { resBindingSet };
    resBindingLayout = gRendererDevice->CreateResourceBindingLayout(ResourceBindingLayoutDesc(sets, 1));
    ASSERT_NE(nullptr, resBindingLayout.Get());

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.vertexShader = mVertexShader;
    pipelineStateDesc.pixelShader = mPixelShader;
    pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
    pipelineStateDesc.vertexLayout = mVertexLayout;
    pipelineStateDesc.resBindingLayout = resBindingLayout;
    pipelineStateDesc.numRenderTargets = 1;
    pipelineStateDesc.rtFormats[0] = ElementFormat::R32G32B32A32_Float;
    pipelineState = gRendererDevice->CreatePipelineState(pipelineStateDesc);
    ASSERT_NE(nullptr, pipelineState.Get());


    ElementFormat formats[] = { ElementFormat::R32G32B32A32_Float };
    BeginTestFrame(VIEWPORT_SIZE, VIEWPORT_SIZE, 1, formats);
    {
        mCommandBuffer->Clear(ClearFlagsColor, 1, nullptr, &CLEAR_COLOR);

        const BufferPtr& vb = mVertexBuffer;
        uint32 stride = sizeof(VertexFormat);
        uint32 offset = 0;
        mCommandBuffer->SetVertexBuffers(1, &vb, &stride, &offset);
        mCommandBuffer->SetIndexBuffer(mIndexBuffer, IndexBufferFormat::Uint16);
        mCommandBuffer->SetResourceBindingLayout(resBindingLayout);
        mCommandBuffer->SetPipelineState(pipelineState);
        mCommandBuffer->BindResources(0, resBindingInstance);

        mCommandBuffer->DrawIndexed(6, 1);
    }
    EndTestFrame();

    VerifyPixelsFloat(0, &customColor.x);
}