/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Source file with scene definitions
 */

#include "PCH.hpp"

#include "BasicScene.hpp"
#include "Common.hpp"

#include "../nfCommon/Math/Math.hpp"

#include <vector>
#include <functional>

using namespace NFE::Math;
using namespace NFE::Common;
using namespace NFE::Renderer;

namespace {

////////////////////////////////////////////////////////////
/// Renderer-related pointers, functions, structures etc ///
////////////////////////////////////////////////////////////

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

/// Helper creators for the Scene

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
        { ElementFormat::Float_32, 3,  0, 0, false, 0 }, // position
        { ElementFormat::Float_32, 2, 12, 0, false, 0 }, // tex-coords
        { ElementFormat::Float_32, 4, 20, 0, false, 0 }, // color
    };

    VertexLayoutDesc vertexLayoutDesc;
    vertexLayoutDesc.elements = vertexLayoutElements;
    vertexLayoutDesc.numElements = 3;
    vertexLayoutDesc.vertexShader = gProgramDesc.vertexShader;
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


/////////////////
/// Subscenes ///
/////////////////

// Basic initialization, additionally to RT & BackBuffer shaders are compiled
// Empty window should be visible
bool CreateSubSceneEmpty()
{
    return CreateShaderProgram(false, false);
}

// Adds vertex buffer creation
// Two colored triangles should be visible
bool CreateSubSceneVertexBuffer()
{
    if (!CreateShaderProgram(false, false))
        return false;

    return CreateVertexBuffer(false);
}

// Adds IndexBuffer creation
// A colored triangle and a colored square should be visible
bool CreateSubSceneIndexBuffer()
{
    if (!CreateShaderProgram(false, false))
        return false;

    if (!CreateVertexBuffer(true))
        return false;

    return CreateIndexBuffer();
}

// Adds constant buffers
// The triangle and the square should rotate
bool CreateSubSceneConstantBuffer()
{
    if (!CreateShaderProgram(true, false))
        return false;

    if (!CreateVertexBuffer(true))
        return false;

    if (!CreateIndexBuffer())
        return false;

    return CreateConstantBuffer();
}

// Add texture support
// The triangle should be rendered checked
bool CreateSubSceneTexture()
{
    if (!CreateShaderProgram(true, true))
        return false;

    if (!CreateVertexBuffer(true))
        return false;

    if (!CreateIndexBuffer())
        return false;

    if (!CreateConstantBuffer())
        return false;

    return CreateTexture();
}


// SubScene container
size_t gHighestAvailableSubScene = 0;
SubSceneArrayType gSubScenes =
{
    CreateSubSceneEmpty,
    CreateSubSceneVertexBuffer,
    CreateSubSceneIndexBuffer,
    CreateSubSceneConstantBuffer,
    CreateSubSceneTexture,
};

/// Utility APIs, used in Main.cpp
bool InitSubScene(size_t sceneId)
{
    return gSubScenes[sceneId]();
}

size_t GetSubSceneCount()
{
    return gSubScenes.size();
}

} // namespace


/////////////////////////////////////////////////
/// BasicScene methods and virtuals overriden ///
/////////////////////////////////////////////////

BasicScene::~BasicScene()
{
    Release();
}

void BasicScene::ReleaseSubsceneResources()
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

bool BasicScene::Init(IDevice* rendererDevice, void* winHandle)
{
    gRendererDevice = rendererDevice;

    gCommandBuffer = gRendererDevice->GetDefaultCommandBuffer();

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

    gCommandBuffer->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);

    BlendStateDesc blendStateDesc;
    blendStateDesc.independent = false;
    blendStateDesc.rtDescs[0].enable = true;
    gBlendState = gRendererDevice->CreateBlendState(blendStateDesc);
    if (!gBlendState)
        return false;

    gCommandBuffer->SetBlendState(gBlendState);

    /// Basic stuff initialized, try to find the highest subscene possible
    for (gHighestAvailableSubScene = gSubScenes.size()-1; ; gHighestAvailableSubScene--)
    {
        if (gSubScenes[gHighestAvailableSubScene]())
            break; // the scene initialized successfully

        if (gHighestAvailableSubScene == 0)
            return false; // we hit the end of our scenes vector, no scene successfully inited

        // nope, release it and continue checking
        ReleaseSubsceneResources();
    }

    return true;
}

bool BasicScene::SwitchSubScene(size_t subScene)
{
    ReleaseSubsceneResources();
    return gSubScenes[subScene]();
}

size_t BasicScene::GetAvailableSubSceneCount()
{
    return gHighestAvailableSubScene;
}

void BasicScene::Draw()
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

void BasicScene::Release()
{
    ReleaseSubsceneResources();
    SAFE_DELETE(gBlendState);
    SAFE_DELETE(gWindowRenderTarget);
    SAFE_DELETE(gWindowBackbuffer);
    gRendererDevice = nullptr;
}
