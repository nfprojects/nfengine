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
#include "nfCommon/Language.hpp"

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

bool TessellationScene::CreateShaderProgram()
{
    std::string vsPath = gShaderPathPrefix + "TessellationVS" + gShaderPathExt;
    mVertexShader.reset(CompileShader(vsPath.c_str(), ShaderType::Vertex, nullptr, 0));
    if (!mVertexShader)
        return false;

    std::string hsPath = gShaderPathPrefix + "TessellationHS" + gShaderPathExt;
    mHullShader.reset(CompileShader(hsPath.c_str(), ShaderType::Hull, nullptr, 0));
    if (!mHullShader)
        return false;

    std::string dsPath = gShaderPathPrefix + "TessellationDS" + gShaderPathExt;
    mDomainShader.reset(CompileShader(dsPath.c_str(), ShaderType::Domain, nullptr, 0));
    if (!mDomainShader)
        return false;

    std::string psPath = gShaderPathPrefix + "TessellationPS" + gShaderPathExt;
    mPixelShader.reset(CompileShader(psPath.c_str(), ShaderType::Pixel, nullptr, 0));
    if (!mPixelShader)
        return false;

    ShaderProgramDesc shaderProgramDesc;
    shaderProgramDesc.vertexShader = mVertexShader.get();
    shaderProgramDesc.pixelShader = mPixelShader.get();
    shaderProgramDesc.hullShader = mHullShader.get();
    shaderProgramDesc.domainShader = mDomainShader.get();
    mShaderProgram.reset(mRendererDevice->CreateShaderProgram(shaderProgramDesc));
    if (!mShaderProgram)
        return false;

    // create binding layout
    mResBindingLayout.reset(mRendererDevice->CreateResourceBindingLayout(ResourceBindingLayoutDesc()));
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
    vbDesc.access = BufferAccess::GPU_ReadOnly;
    vbDesc.size = sizeof(vbData);
    vbDesc.initialData = vbData;
    mVertexBuffer.reset(mRendererDevice->CreateBuffer(vbDesc));
    if (!mVertexBuffer)
        return false;

    VertexLayoutElement vertexLayoutElements[] =
    {
        { ElementFormat::Float_32, 3,  0, 0, false, 0 }, // position
    };

    VertexLayoutDesc vertexLayoutDesc;
    vertexLayoutDesc.elements = vertexLayoutElements;
    vertexLayoutDesc.numElements = 1;
    mVertexLayout.reset(mRendererDevice->CreateVertexLayout(vertexLayoutDesc));
    if (!mVertexLayout)
        return false;

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.blendState.independent = false;
    pipelineStateDesc.blendState.rtDescs[0].enable = true;
    pipelineStateDesc.primitiveType = PrimitiveType::Patch;
    pipelineStateDesc.numControlPoints = 4;
    pipelineStateDesc.vertexLayout = mVertexLayout.get();
    pipelineStateDesc.resBindingLayout = mResBindingLayout.get();
    mPipelineState.reset(mRendererDevice->CreatePipelineState(pipelineStateDesc));
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
    if (!CreateShaderProgram())
        return false;

    return CreateVertexBuffer();
}


/////////////////////////////////////////////////
/// TessellationScene methods and virtuals overriden ///
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
    mVertexLayout.reset();
    mVertexBuffer.reset();

    mPixelShader.reset();
    mHullShader.reset();
    mDomainShader.reset();
    mVertexShader.reset();

    mPipelineState.reset();
    mShaderProgram.reset();
    mResBindingLayout.reset();
}

bool TessellationScene::OnInit(void* winHandle)
{
    // create backbuffer connected with the window
    BackbufferDesc bbDesc;
    bbDesc.width = WINDOW_WIDTH;
    bbDesc.height = WINDOW_HEIGHT;
    bbDesc.windowHandle = winHandle;
    bbDesc.vSync = false;
    mWindowBackbuffer.reset(mRendererDevice->CreateBackbuffer(bbDesc));
    if (!mWindowBackbuffer)
        return false;

    // create rendertarget that will render to the window's backbuffer
    RenderTargetElement rtTarget;
    rtTarget.texture = mWindowBackbuffer.get();
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;
    mWindowRenderTarget.reset(mRendererDevice->CreateRenderTarget(rtDesc));
    if (!mWindowRenderTarget)
        return false;

    return true;
}

void TessellationScene::Draw(float dt)
{
    UNUSED(dt);

    // reset bound resources and set them once again
    mCommandBuffer->Reset();
    mCommandBuffer->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);
    mCommandBuffer->SetScissors(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    mCommandBuffer->SetRenderTarget(mWindowRenderTarget.get());

    int stride = 3 * sizeof(float);
    int offset = 0;

    if (mVertexBuffer)
    {
        IBuffer* vb = mVertexBuffer.get();
        mCommandBuffer->SetVertexBuffers(1, &vb, &stride, &offset);
    }

    if (mResBindingLayout)
        mCommandBuffer->SetResourceBindingLayout(mResBindingLayout.get());

    if (mPipelineState)
        mCommandBuffer->SetPipelineState(mPipelineState.get());

    mCommandBuffer->SetRenderTarget(mWindowRenderTarget.get());

    if (mShaderProgram)
        mCommandBuffer->SetShaderProgram(mShaderProgram.get());

    // clear target
    float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    mCommandBuffer->Clear(NFE_CLEAR_FLAG_TARGET, color);

    // draw
    mCommandBuffer->Draw(4, 1);

    mRendererDevice->Execute(mCommandBuffer->Finish().get());
    mWindowBackbuffer->Present();
}

void TessellationScene::Release()
{
    ReleaseSubsceneResources();
    mWindowRenderTarget.reset();
    mWindowBackbuffer.reset();
    mCommandBuffer = nullptr;
    mRendererDevice = nullptr;
}
