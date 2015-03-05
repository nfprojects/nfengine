/**
 * @file    main.cpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Main source file of renderer test
 */

#include "stdafx.hpp"
#include "../Renderers/RendererInterface/Device.hpp"
#include "../nfCommon/Window.hpp"
#include "../nfCommon/File.hpp"

// TODO: change current directory to nfEngine's root, so the test can be run from any place.
#define D3D11_SHADER_PATH_PREFIX "../../../nfEngine/Renderers/Shaders/D3D11/"

using namespace NFE::Common;
using namespace NFE::Renderer;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

HMODULE gRendererModule = NULL;
IDevice* gRendererDevice = nullptr;
ICommandBuffer* gCommandBuffer = nullptr;

bool InitRenderer()
{
    gRendererModule = LoadLibrary(L"nfRendererD3D11.dll");
    if (gRendererModule == NULL)
        return false;

    auto proc = (RendererInitFunc)GetProcAddress(gRendererModule, RENDERER_INIT_FUNC);
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
    File file(path, AccessMode::Read);
    size_t size = file.GetSize();
    std::vector<char> str(size + 1);
    file.Read(str.data(), size);
    str[size] = '\0';

    ShaderDesc desc;
    desc.code = str.data();
    desc.type = type;
    desc.name = path;
    return gRendererDevice->CreateShader(desc);
}

void ReleaseRenderer()
{
    if (gRendererDevice != nullptr)
    {
        gRendererDevice = nullptr;
        auto proc = (RendererReleaseFunc)GetProcAddress(gRendererModule, "Release");
        if (proc == NULL)
            return;
        proc();
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    Window window;
    window.SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    window.SetTitle(L"nfEngine Renderer Test");
    window.Open();

    if (!InitRenderer())
        return 1;

    RenderTargetDesc rtDesc;
    rtDesc.width = WINDOW_WIDTH;
    rtDesc.height = WINDOW_HEIGHT;
    rtDesc.windowHandle = window.GetHandle();
    IRenderTarget* windowRenderTarget = gRendererDevice->CreateRenderTarget(rtDesc);

    ShaderProgramDesc programDesc;
    programDesc.vertexShader = CompileShader(D3D11_SHADER_PATH_PREFIX "TestVS.hlsl",
                               ShaderType::Vertex);
    programDesc.pixelShader = CompileShader(D3D11_SHADER_PATH_PREFIX "TestPS.hlsl", ShaderType::Pixel);
    IShaderProgram* shaderProgram = gRendererDevice->CreateShaderProgram(programDesc);

    float vbData[] =
    {
        // pos.xyz, color.rgba
        0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.0f,    0.0f, 0.0f, 1.0f, 1.0f,

        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
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
        { ElementFormat::Float_32, 4 }, // color
    };
    VertexLayoutDesc vertexLayoutDesc = { vertexLayoutElements, 2, programDesc.vertexShader };
    IVertexLayout* vertexLayout = gRendererDevice->CreateVertexLayout(vertexLayoutDesc);


    BlendStateDesc blendStateDesc;
    blendStateDesc.independent = false;
    blendStateDesc.rtDescs[0].enable = true;
    IBlendState* blendState = gRendererDevice->CreateBlendState(blendStateDesc);


    // RENDERING LOOP =============================================================================

    gCommandBuffer->SetBlendState(blendState);
    gCommandBuffer->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);

    while (!window.IsClosed())
    {
        window.ProcessMessages();

        float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        gCommandBuffer->SetRenderTargets(&windowRenderTarget, 1);
        gCommandBuffer->Clear(color);
        gCommandBuffer->SetShaderProgram(shaderProgram);
        gCommandBuffer->SetVertexLayout(vertexLayout);

        int stride = 7 * sizeof(float);
        int offset = 0;
        gCommandBuffer->SetVertexBuffers(1, &vertexBuffer, &stride, &offset);

        gCommandBuffer->Draw(PrimitiveType::Triangles, 6);

        windowRenderTarget->Present();
    }

    delete blendState;
    delete vertexLayout;
    delete vertexBuffer;

    ReleaseRenderer();
    return 0;
}
