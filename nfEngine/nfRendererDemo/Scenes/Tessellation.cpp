/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of TessellationScene functions
 */

#include "../PCH.hpp"

#include "Tessellation.hpp"
#include "../Common.hpp"

#include "nfCommon/Math/Math.hpp"
#include "nfCommon/Math/Matrix.hpp"


#include <vector>
#include <functional>


using namespace NFE;
using namespace NFE::Math;
using namespace NFE::Renderer;

namespace {

struct VertexCBuffer
{
    Matrix viewMatrix;
};

struct PixelCBuffer
{
    Matrix viewMatrix;
};

} // namespace

/// Helper creators for the Scene

bool TessellationScene::LoadShaders()
{
    std::string vsPath = gShaderPathPrefix + "TessellationVS" + gShaderPathExt;
    mVertexShader = CompileShader(vsPath.c_str(), ShaderType::Vertex, nullptr, 0);
    if (!mVertexShader)
        return false;

    std::string hsPath = gShaderPathPrefix + "TessellationHS" + gShaderPathExt;
    mHullShader = CompileShader(hsPath.c_str(), ShaderType::Hull, nullptr, 0);
    if (!mHullShader)
        return false;

    std::string dsPath = gShaderPathPrefix + "TessellationDS" + gShaderPathExt;
    mDomainShader = CompileShader(dsPath.c_str(), ShaderType::Domain, nullptr, 0);
    if (!mDomainShader)
        return false;

    std::string psPath = gShaderPathPrefix + "TessellationPS" + gShaderPathExt;
    mPixelShader = CompileShader(psPath.c_str(), ShaderType::Pixel, nullptr, 0);
    if (!mPixelShader)
        return false;

    // create binding layout
    mResBindingLayout = mRendererDevice->CreateResourceBindingLayout(ResourceBindingLayoutDesc());
    if (!mResBindingLayout)
        return false;

    return true;
}

bool TessellationScene::CreateVertexBuffer()
{
    // create vertex buffers
    float vbData[] =
    {
        // Bezier curve control points
        -1.0f, -0.8f, 0.0f,
         4.0f, -1.0f, 0.0f,
        -4.0f,  1.0f, 0.0f,
         1.0f,  0.8f, 0.0f,
    };

    BufferDesc vbDesc;
    vbDesc.type = BufferType::Vertex;
    vbDesc.mode = BufferMode::Static;
    vbDesc.size = sizeof(vbData);
    vbDesc.initialData = vbData;
    mVertexBuffer = mRendererDevice->CreateBuffer(vbDesc);
    if (!mVertexBuffer)
        return false;

    VertexLayoutElement vertexLayoutElements[] =
    {
        { ElementFormat::R32G32B32_Float, 0, 0, false, 0 }, // position
    };

    VertexLayoutDesc vertexLayoutDesc;
    vertexLayoutDesc.elements = vertexLayoutElements;
    vertexLayoutDesc.numElements = 1;
    mVertexLayout = mRendererDevice->CreateVertexLayout(vertexLayoutDesc);
    if (!mVertexLayout)
        return false;

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.rtFormats[0] = mBackbufferFormat;
    pipelineStateDesc.vertexShader = mVertexShader;
    pipelineStateDesc.pixelShader = mPixelShader;
    pipelineStateDesc.hullShader = mHullShader;
    pipelineStateDesc.domainShader = mDomainShader;
    pipelineStateDesc.blendState.independent = false;
    pipelineStateDesc.blendState.rtDescs[0].enable = true;
    pipelineStateDesc.primitiveType = PrimitiveType::Patch;
    pipelineStateDesc.numControlPoints = 4;
    pipelineStateDesc.vertexLayout = mVertexLayout;
    pipelineStateDesc.resBindingLayout = mResBindingLayout;
    mPipelineState = mRendererDevice->CreatePipelineState(pipelineStateDesc);
    if (!mPipelineState)
        return false;

    return true;
}

/////////////////
/// Subscenes ///
/////////////////

// Basic initialization, additionally to RT & BackBuffer shaders are compiled
// Empty window should be visible
bool TessellationScene::CreateSubSceneBezierLine()
{
    if (!LoadShaders())
        return false;

    return CreateVertexBuffer();
}


/////////////////////////////////////////////////
/// TessellationScene methods and virtuals overridden ///
/////////////////////////////////////////////////

TessellationScene::TessellationScene()
    : Scene("Tessellation")
{
    RegisterSubScene(std::bind(&TessellationScene::CreateSubSceneBezierLine, this), "BezierLine");
}

TessellationScene::~TessellationScene()
{
    Release();
}

void TessellationScene::ReleaseSubsceneResources()
{
    // clear resources
    mVertexLayout.Reset();
    mVertexBuffer.Reset();

    mPixelShader.Reset();
    mHullShader.Reset();
    mDomainShader.Reset();
    mVertexShader.Reset();

    mPipelineState.Reset();
    mResBindingLayout.Reset();
}

bool TessellationScene::OnInit(void* winHandle)
{
    // create backbuffer connected with the window
    BackbufferDesc bbDesc;
    bbDesc.width = WINDOW_WIDTH;
    bbDesc.height = WINDOW_HEIGHT;
    bbDesc.format = mBackbufferFormat;
    bbDesc.windowHandle = winHandle;
    bbDesc.vSync = false;
    mWindowBackbuffer = mRendererDevice->CreateBackbuffer(bbDesc);
    if (!mWindowBackbuffer)
        return false;

    // create rendertarget that will render to the window's backbuffer
    RenderTargetElement rtTarget;
    rtTarget.texture = mWindowBackbuffer;
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;
    mWindowRenderTarget = mRendererDevice->CreateRenderTarget(rtDesc);
    if (!mWindowRenderTarget)
        return false;

    return true;
}

void TessellationScene::Draw(float dt)
{
    NFE_UNUSED(dt);

    // reset bound resources and set them once again
    mCommandBuffer->Begin();
    mCommandBuffer->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);
    mCommandBuffer->SetScissors(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    mCommandBuffer->SetRenderTarget(mWindowRenderTarget);

    int stride = 3 * sizeof(float);
    int offset = 0;

    if (mVertexBuffer)
    {
        const BufferPtr vertexBuffers[] = { mVertexBuffer };
        mCommandBuffer->SetVertexBuffers(1, vertexBuffers, &stride, &offset);
    }

    if (mResBindingLayout)
        mCommandBuffer->SetResourceBindingLayout(mResBindingLayout);

    if (mPipelineState)
        mCommandBuffer->SetPipelineState(mPipelineState);

    mCommandBuffer->SetRenderTarget(mWindowRenderTarget);

    // clear target
    const Float4 color(0.0f, 0.0f, 0.0f, 1.0f);
    mCommandBuffer->Clear(ClearFlagsColor, 1, nullptr, &color);

    // draw
    mCommandBuffer->Draw(4, 1);

    CommandListID commandList = mCommandBuffer->Finish();
    mRendererDevice->Execute(commandList);
    mWindowBackbuffer->Present();
    mRendererDevice->FinishFrame();
}

void TessellationScene::Release()
{
    ReleaseSubsceneResources();
    mWindowRenderTarget.Reset();
    mWindowBackbuffer.Reset();
    mCommandBuffer.Reset();
    mRendererDevice = nullptr;
}
