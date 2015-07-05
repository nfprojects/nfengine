/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Definitions of Renderer utility functions used by Scenes
 */

#include "PCH.hpp"

#include "RendererUtils.hpp"

#include "Common.hpp"

#include "../Renderers/RendererInterface/Device.hpp"
#include "../nfCommon/Library.hpp"
#include "../nfCommon/Window.hpp"
#include "../nfCommon/Math/Math.hpp"

using namespace NFE::Math;
using namespace NFE::Common;
using namespace NFE::Renderer;


/// Interfaces to test
IDevice* gRendererDevice = nullptr;
ICommandBuffer* gCommandBuffer = nullptr;
IBackbuffer* gWindowBackbuffer = nullptr;
IRenderTarget* gWindowRenderTarget = nullptr;
IShaderProgram* gShaderProgram = nullptr;
IBuffer* gVertexBuffer = nullptr;
IVertexLayout* gVertexLayout = nullptr;
IBlendState* gBlendState = nullptr;
IBuffer* gIndexBuffer = nullptr;
IBuffer* gConstantBuffer = nullptr;
ISampler* gSampler = nullptr;
ITexture* gTexture = nullptr;

ShaderProgramDesc gProgramDesc; // VertexLayout requires vertex shader access
float gAngle = 0.0f; // for objects rotation, used in Constant Buffer scenes
Library gRendererLib;

struct VertexCBuffer
{
    Matrix viewMatrix;
};

struct PixelCBuffer
{
    Matrix viewMatrix;
};

#define D3D11_SHADER_PATH_PREFIX "nfEngine/nfRendererDemo/Shaders/D3D11/"
#define SAFE_DELETE(x) do { delete x; x = nullptr; } while(0)

IShader* CompileShader(const char* path, ShaderType type, ShaderMacro* macros, size_t macrosNum)
{
    ShaderDesc desc;
    desc.type = type;
    desc.path = path;
    desc.macros = macros;
    desc.macrosNum = macrosNum;
    return gRendererDevice->CreateShader(desc);
}

bool InitRenderer(const std::string& renderer)
{
    if (!gRendererLib.Open(renderer))
        return false;

    RendererInitFunc proc;
    if (!gRendererLib.GetSymbol(RENDERER_INIT_FUNC, proc))
        return false;

    gRendererDevice = proc();
    if (gRendererDevice == nullptr)
        return false;

    gCommandBuffer = gRendererDevice->GetDefaultCommandBuffer();
    return true;
}

bool InitRenderTarget(void* winHandle)
{
    // create backbuffer connected with the window
    BackbufferDesc bbDesc;
    bbDesc.width = WINDOW_WIDTH;
    bbDesc.height = WINDOW_HEIGHT;
    bbDesc.windowHandle = winHandle;
    bbDesc.vSync = true;
    gWindowBackbuffer = gRendererDevice->CreateBackbuffer(bbDesc);
    if (!gWindowBackbuffer)
        return false;

    // create rendertarget that will render to the window's backbuffer
    RenderTargetElement rtTarget;
    rtTarget.texture = gWindowBackbuffer;
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;
    gWindowRenderTarget = gRendererDevice->CreateRenderTarget(rtDesc);
    if (!gWindowRenderTarget)
        return false;

    return true;
}

bool InitBlendState(bool enabled)
{
    BlendStateDesc blendStateDesc;
    blendStateDesc.independent = false;
    blendStateDesc.rtDescs[0].enable = enabled;
    gBlendState = gRendererDevice->CreateBlendState(blendStateDesc);
    if (!gBlendState)
        return false;

    gCommandBuffer->SetBlendState(gBlendState);
    return true;
}

void ReleaseResources()
{
    SAFE_DELETE(gConstantBuffer);
    SAFE_DELETE(gSampler);
    SAFE_DELETE(gTexture);
    SAFE_DELETE(gVertexLayout);
    SAFE_DELETE(gIndexBuffer);
    SAFE_DELETE(gVertexBuffer);
    SAFE_DELETE(gProgramDesc.vertexShader);
    SAFE_DELETE(gProgramDesc.pixelShader);
}

void ReleaseBlendState()
{
    SAFE_DELETE(gBlendState);
}

void ReleaseRenderTarget()
{
    SAFE_DELETE(gWindowRenderTarget);
    SAFE_DELETE(gWindowBackbuffer);
}

void ReleaseRenderer()
{
    if (gRendererDevice != nullptr)
    {
        gRendererDevice = nullptr;
        RendererReleaseFunc proc;
        if (!gRendererLib.GetSymbol("Release", proc))
            return;
        proc();
    }

    gRendererLib.Close();
}


/// Helper creators for Scenes

bool CreateShaderProgram(bool useCBuffer, bool useTexture)
{
    ShaderMacro vsMacro[] = { { "USE_CBUFFER", useCBuffer ? "1" : "0" } };
    gProgramDesc.vertexShader = CompileShader(D3D11_SHADER_PATH_PREFIX "TestVS.hlsl",
        ShaderType::Vertex, vsMacro, 1);

    ShaderMacro psMacro[] = { { "USE_TEXTURE", useTexture ? "1" : "0" } };
    gProgramDesc.pixelShader = CompileShader(D3D11_SHADER_PATH_PREFIX "TestPS.hlsl",
        ShaderType::Pixel, psMacro, 1);
    if (!gProgramDesc.vertexShader)
        return false;
    if (!gProgramDesc.pixelShader)
        return false;

    gShaderProgram = gRendererDevice->CreateShaderProgram(gProgramDesc);
    if (!gShaderProgram)
        return false;

    return true;
}

bool CreateVertexBuffer(bool withExtraVert)
{
    // create vertex buffers
    float vbData[] =
    {
        /// Vertex structure: pos.xyz, texCoord.uv, color.rgba

        0.5f, -0.5f, 0.0f, 10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.0f, 10.0f, 10.0f, 0.0f, 0.0f, 1.0f, 1.0f,

        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    };

    // create vertex buffer
    float vbDataExtra[] =
    {
        /// Vertex structure: pos.xyz, texCoord.uv, color.rgba

        0.5f, -0.5f, 0.0f, 10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.0f, 10.0f, 10.0f, 0.0f, 0.0f, 1.0f, 1.0f,

        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    };

    BufferDesc vbDesc;
    vbDesc.type = BufferType::Vertex;
    vbDesc.access = BufferAccess::GPU_ReadOnly;
    vbDesc.size = withExtraVert ? sizeof(vbDataExtra) : sizeof(vbData);
    vbDesc.initialData = withExtraVert ? vbDataExtra : vbData;
    gVertexBuffer = gRendererDevice->CreateBuffer(vbDesc);
    if (!gVertexBuffer)
        return false;

    VertexLayoutElement vertexLayoutElements[] =
    {
        { ElementFormat::Float_32, 3 }, // position
        { ElementFormat::Float_32, 2 }, // tex-coords
        { ElementFormat::Float_32, 4 }, // color
    };
    VertexLayoutDesc vertexLayoutDesc = { vertexLayoutElements, 3, gProgramDesc.vertexShader };
    gVertexLayout = gRendererDevice->CreateVertexLayout(vertexLayoutDesc);
    if (!gVertexLayout)
        return false;

    return true;
}

bool CreateIndexBuffer()
{
    // create index buffer
    uint16 ibData[] =
    {
        0, 1, 2,
        3, 4, 5,
        5, 4, 6
    };

    BufferDesc ibDesc;
    ibDesc.type = BufferType::Index;
    ibDesc.access = BufferAccess::GPU_ReadOnly;
    ibDesc.size = sizeof(ibData);
    ibDesc.initialData = ibData;
    gIndexBuffer = gRendererDevice->CreateBuffer(ibDesc);
    if (!gIndexBuffer)
        return false;

    return true;
}

bool CreateConstantBuffer()
{
    gAngle = 0.0f;

    BufferDesc vertexCBufferDesc;
    vertexCBufferDesc.type = BufferType::Constant;
    vertexCBufferDesc.access = BufferAccess::CPU_Write;
    vertexCBufferDesc.size = sizeof(VertexCBuffer);
    gConstantBuffer = gRendererDevice->CreateBuffer(vertexCBufferDesc);
    if (!gConstantBuffer)
        return false;

    return true;
}

bool CreateTexture()
{
    SamplerDesc samplerDesc;
    gSampler = gRendererDevice->CreateSampler(samplerDesc);
    if (!gSampler)
        return false; // there's no need for textures if we cannot sample them

    uint32_t bitmap[] = { 0xFFFFFFFF, 0, 0, 0xFFFFFFFF };
    TextureDataDesc textureDataDesc;
    textureDataDesc.data = bitmap;
    textureDataDesc.lineSize = 2 * sizeof(uint32_t);
    textureDataDesc.sliceSize = 4 * sizeof(uint32_t);

    TextureDesc textureDesc;
    textureDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER;
    textureDesc.format = ElementFormat::Uint_8_norm;
    textureDesc.texelSize = 4;
    textureDesc.width = 2;
    textureDesc.height = 2;
    textureDesc.mipmaps = 1;
    textureDesc.dataDesc = &textureDataDesc;
    textureDesc.layers = 1;
    gTexture = gRendererDevice->CreateTexture(textureDesc);
    if (!gTexture)
        return false;

    return true;
}


/// Drawers

void PreDrawLoop()
{
    gCommandBuffer->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);
}

void Draw()
{
    gAngle += 0.03f;

    if (gConstantBuffer)
    {
        VertexCBuffer vertexCBufferData;
        vertexCBufferData.viewMatrix = MatrixRotationNormal(Vector(0.0f, 0.0f, 1.0f), gAngle);
        gCommandBuffer->WriteBuffer(gConstantBuffer, 0, sizeof(VertexCBuffer), &vertexCBufferData);
    }

    float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    gCommandBuffer->SetRenderTarget(gWindowRenderTarget);
    gCommandBuffer->Clear(NFE_CLEAR_FLAG_TARGET, color);
    if (gShaderProgram) gCommandBuffer->SetShaderProgram(gShaderProgram);
    if (gVertexLayout) gCommandBuffer->SetVertexLayout(gVertexLayout);

    int stride = 9 * sizeof(float);
    int offset = 0;
    if (gIndexBuffer) gCommandBuffer->SetIndexBuffer(gIndexBuffer, IndexBufferFormat::Uint16);
    if (gVertexBuffer) gCommandBuffer->SetVertexBuffers(1, &gVertexBuffer, &stride, &offset);
    if (gConstantBuffer) gCommandBuffer->SetConstantBuffers(&gConstantBuffer, 1, ShaderType::Vertex);
    if (gTexture) gCommandBuffer->SetTextures(&gTexture, 1, ShaderType::Pixel);
    if (gSampler) gCommandBuffer->SetSamplers(&gSampler, 1, ShaderType::Pixel);

    if (gIndexBuffer)
        gCommandBuffer->DrawIndexed(PrimitiveType::Triangles, 9);
    else if (gVertexBuffer)
        gCommandBuffer->Draw(PrimitiveType::Triangles, 6);

    gWindowBackbuffer->Present();
}
