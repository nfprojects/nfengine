/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Main source file of renderer test
 */

#include "PCH.hpp"
#include "../Renderers/RendererInterface/Device.hpp"
#include "../nfCommon/Window.hpp"
#include "../nfCommon/File.hpp"
#include "../nfCommon/Library.hpp"
#include "../nfCommon/Math/Math.hpp"
#include "../nfCommon/FileSystem.hpp"

#define D3D11_SHADER_PATH_PREFIX "nfEngine/nfRendererDemo/Shaders/D3D11/"

using namespace NFE::Math;
using namespace NFE::Common;
using namespace NFE::Renderer;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

const std::string NFRENDERER_D3D11_DLL("nfRendererD3D11.dll");

Library gRendererLib;
IDevice* gRendererDevice = nullptr;
ICommandBuffer* gCommandBuffer = nullptr;

// flags for checking if features are implemented
bool gVertexLayout = true;
bool gShaders = true;
bool gBlendState = true;
bool gTextures = true;
bool gConstantBuffers = true;

struct VertexCBuffer
{
    Matrix viewMatrix;
};

struct PixelCBuffer
{
    Matrix viewMatrix;
};

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
        RendererReleaseFunc proc;
        if (!gRendererLib.GetSymbol("Release", proc))
            return;
        proc();
    }

    gRendererLib.Close();
}

int main(int argc, char* argv[])
{
    std::string execPath = NFE::Common::FileSystem::GetExecutablePath();
    NFE::Common::FileSystem::ChangeDirectory(execPath + "/../../../..");

    Window window;
    window.SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    window.SetTitle("nfEngine Renderer Test");
    window.Open();

    /// select renderer to use - the default will be D3D11 renderer
    std::string rend;
    if (argc < 2)
        rend = NFRENDERER_D3D11_DLL;
    else
        rend = argv[1];

    if (!InitRenderer(rend))
        return 1;

    // create backbuffer connected with the window
    BackbufferDesc bbDesc;
    bbDesc.width = WINDOW_WIDTH;
    bbDesc.height = WINDOW_HEIGHT;
    bbDesc.windowHandle = static_cast<void*>(window.GetHandle());
    bbDesc.vSync = true;
    IBackbuffer* windowBackbuffer = gRendererDevice->CreateBackbuffer(bbDesc);
    if (!windowBackbuffer)
        return 1;

    // create rendertarget that will render to the window's backbuffer
    RenderTargetElement rtTarget;
    rtTarget.texture = windowBackbuffer;
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;
    IRenderTarget* windowRenderTarget = gRendererDevice->CreateRenderTarget(rtDesc);
    if (!windowRenderTarget)
        return 1;


    ShaderProgramDesc programDesc;
    programDesc.vertexShader = CompileShader(D3D11_SHADER_PATH_PREFIX "TestVS.hlsl",
                               ShaderType::Vertex);
    programDesc.pixelShader = CompileShader(D3D11_SHADER_PATH_PREFIX "TestPS.hlsl", ShaderType::Pixel);
    IShaderProgram* shaderProgram = gRendererDevice->CreateShaderProgram(programDesc);
    if (!shaderProgram)
        gShaders = false;


    // create vertex buffer

    float vbData[] =
    {
        /// Vertex structure: pos.xyz, texCoord.uv, color.rgba

        0.5f, -0.5f, 0.0f,   10.0f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,    1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.0f,    10.0f, 10.0f,  0.0f, 0.0f, 1.0f, 1.0f,

        0.0f, 0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,  0.0f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,
    };

    BufferDesc vbDesc;
    vbDesc.type = BufferType::Vertex;
    vbDesc.access = BufferAccess::GPU_ReadOnly;
    vbDesc.size = sizeof(vbData);
    vbDesc.initialData = vbData;
    IBuffer* vertexBuffer = gRendererDevice->CreateBuffer(vbDesc);


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
    IBuffer* indexBuffer = gRendererDevice->CreateBuffer(ibDesc);


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
    if (!blendState)
        gBlendState = false;

    // TEXTURE ====================================================================================

    SamplerDesc samplerDesc;
    ISampler* sampler = gRendererDevice->CreateSampler(samplerDesc);
    if (!sampler)
        gTextures = false; // there's no need for textures if we cannot sample them

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
    if (!texture)
        gTextures = false;

    BufferDesc vertexCBufferDesc;
    vertexCBufferDesc.type = BufferType::Constant;
    vertexCBufferDesc.access = BufferAccess::CPU_Write;
    vertexCBufferDesc.size = sizeof(VertexCBuffer);
    IBuffer* constantBuffer = gRendererDevice->CreateBuffer(vertexCBufferDesc);
    if (!constantBuffer)
        gConstantBuffers = false;

    // RENDERING LOOP =============================================================================

    if (gBlendState) gCommandBuffer->SetBlendState(blendState);
    gCommandBuffer->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);

    float angle = 0.0f;

    while (!window.IsClosed())
    {
        window.ProcessMessages();
        angle += 0.03f;

        if (gConstantBuffers)
        {
            VertexCBuffer vertexCBufferData;
            vertexCBufferData.viewMatrix = MatrixRotationNormal(Vector(0.0f, 0.0f, 1.0f), angle);
            gCommandBuffer->WriteBuffer(constantBuffer, 0, sizeof(VertexCBuffer), &vertexCBufferData);
        }

        float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        gCommandBuffer->SetRenderTarget(windowRenderTarget);
        gCommandBuffer->Clear(NFE_CLEAR_FLAG_TARGET, color);
        if (gShaders) gCommandBuffer->SetShaderProgram(shaderProgram);
        gCommandBuffer->SetVertexLayout(vertexLayout);

        int stride = 9 * sizeof(float);
        int offset = 0;
        gCommandBuffer->SetIndexBuffer(indexBuffer, IndexBufferFormat::Uint16);
        gCommandBuffer->SetVertexBuffers(1, &vertexBuffer, &stride, &offset);
        if (gConstantBuffers) gCommandBuffer->SetConstantBuffers(&constantBuffer, 1, ShaderType::Vertex);
        if (gTextures) gCommandBuffer->SetTextures(&texture, 1, ShaderType::Pixel);
        if (gTextures) gCommandBuffer->SetSamplers(&sampler, 1, ShaderType::Pixel);

        gCommandBuffer->DrawIndexed(PrimitiveType::Triangles, 9);

        windowBackbuffer->Present();
    }

    delete constantBuffer;
    delete sampler;
    delete texture;
    delete blendState;
    delete vertexLayout;
    delete indexBuffer;
    delete vertexBuffer;
    delete programDesc.vertexShader;
    delete programDesc.pixelShader;
    delete windowRenderTarget;
    delete windowBackbuffer;

    ReleaseRenderer();
    return 0;
}
