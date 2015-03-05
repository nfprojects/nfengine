#include "stdafx.hpp"
#include "../Renderers/RendererInterface/Device.hpp"

#define D3D11_SHADER_PATH_PREFIX "../../../nfEngine/Renderers/Shaders/D3D11/"

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

    auto proc = (RendererInitFunc)GetProcAddress(gRendererModule, "Init");
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
    std::ifstream file(path);
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    ShaderDesc desc;
    desc.code = str.c_str();
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

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    HWND hWnd;
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = L"WindowClass";
    RegisterClassEx(&wc);

    RECT wr = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    hWnd = CreateWindowEx(NULL, L"WindowClass", L"nfEngine Renderer Test", WS_OVERLAPPEDWINDOW,
                          10, 10, wr.right - wr.left, wr.bottom - wr.top,
                          NULL, NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);

    if (!InitRenderer())
        return 1;


    RenderTargetDesc rtDesc;
    rtDesc.width = WINDOW_WIDTH;
    rtDesc.height = WINDOW_HEIGHT;
    rtDesc.windowHandle = static_cast<void*>(hWnd);
    IRenderTarget* windowRenderTarget = gRendererDevice->CreateRenderTarget(rtDesc);

    ShaderProgramDesc programDesc;
    programDesc.vertexShader = CompileShader(D3D11_SHADER_PATH_PREFIX "TestVS.hlsl", ShaderType::Vertex);
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

    MSG msg;
    while (TRUE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                break;
        }
        else
        {
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
    }

    delete blendState;
    delete vertexLayout;
    delete vertexBuffer;

    ReleaseRenderer();
    return msg.wParam;
}
