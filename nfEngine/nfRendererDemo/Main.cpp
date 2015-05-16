/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Main source file of renderer test
 */

#include "PCH.hpp"
#include "../Renderers/RendererInterface/Device.hpp"
#include "../nfCommon/Window.hpp"
#include "../nfCommon/File.hpp"
#include "../nfCommon/Library.hpp"
#include "../nfCommon/Math/Math.hpp"

// TODO: change current directory to nfEngine's root, so the test can be run from any place.
#define D3D11_SHADER_PATH_PREFIX "../../../nfEngine/nfRendererDemo/Shaders/D3D11/"

using namespace NFE::Math;
using namespace NFE::Common;
using namespace NFE::Renderer;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

Library gRendererLib;
IDevice* gRendererDevice = nullptr;
ICommandBuffer* gCommandBuffer = nullptr;

struct VertexCBuffer
{
    Matrix viewMatrix;
};

struct PixelCBuffer
{
    Matrix viewMatrix;
};

bool InitRenderer()
{
    if (!gRendererLib.Open("nfRendererD3D11.dll"))
        return false;

    auto proc = static_cast<RendererInitFunc>(gRendererLib.GetSymbol(RENDERER_INIT_FUNC));
    if (proc == NULL)
        return false;

    gRendererDevice = proc();
    if (gRendererDevice == nullptr)
        return false;

    gCommandBuffer = gRendererDevice->GetDefaultCommandBuffer();
    return true;
}

IShader* CompileShader(const char* path, ShaderType type)
{
    ShaderDesc desc;
    desc.type = type;
    desc.path = path;
    return gRendererDevice->CreateShader(desc);
}

void ReleaseRenderer()
{
    if (gRendererDevice != nullptr)
    {
        gRendererDevice = nullptr;
        auto proc = static_cast<RendererReleaseFunc>(gRendererLib.GetSymbol("Release"));
        if (proc == NULL)
            return;
        proc();
    }

    gRendererLib.Close();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    Window window;
    window.SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    window.SetTitle("nfEngine Renderer Test");
    window.Open();

    if (!InitRenderer())
        return 1;

    // create backbuffer connected with the window
    BackbufferDesc bbDesc;
    bbDesc.width = WINDOW_WIDTH;
    bbDesc.height = WINDOW_HEIGHT;
    bbDesc.windowHandle = static_cast<void*>(window.GetHandle());
    bbDesc.vSync = true;
    IBackbuffer* windowBackbuffer = gRendererDevice->CreateBackbuffer(bbDesc);

    // create rendertarget that will render to the window's backbuffer
    RenderTargetElement rtTarget;
    rtTarget.texture = windowBackbuffer;
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;
    IRenderTarget* windowRenderTarget = gRendererDevice->CreateRenderTarget(rtDesc);


    ShaderProgramDesc programDesc;
    programDesc.vertexShader = CompileShader(D3D11_SHADER_PATH_PREFIX "TestVS.hlsl",
                               ShaderType::Vertex);
    programDesc.pixelShader = CompileShader(D3D11_SHADER_PATH_PREFIX "TestPS.hlsl", ShaderType::Pixel);
    IShaderProgram* shaderProgram = gRendererDevice->CreateShaderProgram(programDesc);

    float vbData[] =
    {
        /// Vertex structure: pos.xyz, texCoord.uv, color.rgba

        0.5f, -0.5f, 0.0f,   10.0f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,    1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.0f,    10.0f, 10.0f,  0.0f, 0.0f, 1.0f, 1.0f,

        0.0f, 0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    };

    BufferDesc vbDesc;
    vbDesc.type = BufferType::Vertex;
    vbDesc.access = BufferAccess::GPU_ReadOnly;
    vbDesc.size = sizeof(vbData);
    vbDesc.initialData = vbData;
    IBuffer* vertexBuffer = gRendererDevice->CreateBuffer(vbDesc);

    VertexLayoutElement vertexLayoutElements[] =
    {
        { ElementFormat::Float_32, 3 }, // position
        { ElementFormat::Float_32, 2 }, // tex-coords
        { ElementFormat::Float_32, 4 }, // color
    };
    VertexLayoutDesc vertexLayoutDesc = { vertexLayoutElements, 3, programDesc.vertexShader };
    IVertexLayout* vertexLayout = gRendererDevice->CreateVertexLayout(vertexLayoutDesc);


    BlendStateDesc blendStateDesc;
    blendStateDesc.independent = false;
    blendStateDesc.rtDescs[0].enable = true;
    IBlendState* blendState = gRendererDevice->CreateBlendState(blendStateDesc);

    // TEXTURE ====================================================================================

    SamplerDesc samplerDesc;
    ISampler* sampler = gRendererDevice->CreateSampler(samplerDesc);

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
    ITexture* texture = gRendererDevice->CreateTexture(textureDesc);

    BufferDesc vertexCBufferDesc;
    vertexCBufferDesc.type = BufferType::Constant;
    vertexCBufferDesc.access = BufferAccess::CPU_Write;
    vertexCBufferDesc.size = sizeof(VertexCBuffer);
    IBuffer* constantBuffer = gRendererDevice->CreateBuffer(vertexCBufferDesc);

    // RENDERING LOOP =============================================================================

    gCommandBuffer->SetBlendState(blendState);
    gCommandBuffer->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);

    float angle = 0.0f;

    while (!window.IsClosed())
    {
        window.ProcessMessages();
        angle += 0.03f;

        VertexCBuffer vertexCBufferData;
        vertexCBufferData.viewMatrix = MatrixRotationNormal(Vector(0.0f, 0.0f, 1.0f), angle);
        gCommandBuffer->WriteBuffer(constantBuffer, 0, sizeof(VertexCBuffer), &vertexCBufferData);

        float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        gCommandBuffer->SetRenderTarget(windowRenderTarget);
        gCommandBuffer->Clear(color);
        gCommandBuffer->SetShaderProgram(shaderProgram);
        gCommandBuffer->SetVertexLayout(vertexLayout);

        int stride = 9 * sizeof(float);
        int offset = 0;
        gCommandBuffer->SetVertexBuffers(1, &vertexBuffer, &stride, &offset);
        gCommandBuffer->SetConstantBuffers(&constantBuffer, 1, ShaderType::Vertex);
        gCommandBuffer->SetTextures(&texture, 1, ShaderType::Pixel);
        gCommandBuffer->SetSamplers(&sampler, 1, ShaderType::Pixel);

        gCommandBuffer->Draw(PrimitiveType::Triangles, 6);

        windowBackbuffer->Present();
    }

    delete constantBuffer;
    delete sampler;
    delete texture;
    delete blendState;
    delete vertexLayout;
    delete vertexBuffer;
    delete programDesc.vertexShader;
    delete programDesc.pixelShader;
    delete windowRenderTarget;
    delete windowBackbuffer;

    ReleaseRenderer();
    return 0;
}
