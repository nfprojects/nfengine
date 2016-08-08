/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  DepthStencil scene definition
 */

#include "../PCH.hpp"

#include "DepthStencil.hpp"
#include "../Common.hpp"

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

} // namespace

DepthStencilScene::DepthStencilScene()
    : Scene("DepthStencil")
{
    RegisterSubScene(std::bind(&DepthStencilScene::CreateSubSceneNoDepthBuffer, this),
                     "NoDepthBuffer");
    RegisterSubScene(std::bind(&DepthStencilScene::CreateSubSceneDepthBuffer, this),
                     "DepthBuffer");
    RegisterSubScene(std::bind(&DepthStencilScene::CreateSubSceneDepthStencilBuffer, this),
                     "DepthStencilBuffer");
}

DepthStencilScene::~DepthStencilScene()
{
    Release();
}

bool DepthStencilScene::CreateBasicResources(bool withStencil)
{
    // create rendertarget that will render to the window's backbuffer
    RenderTargetElement rtTarget;
    rtTarget.texture = mWindowBackbuffer.get();
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;
    rtDesc.depthBuffer = mDepthBuffer.get();
    rtDesc.debugName = "DepthStencilScene::mWindowRenderTarget";
    mWindowRenderTarget.reset(mRendererDevice->CreateRenderTarget(rtDesc));
    if (!mWindowRenderTarget)
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
    mVertexLayout.reset(mRendererDevice->CreateVertexLayout(vertexLayoutDesc));
    if (!mVertexLayout)
        return false;

    ShaderMacro vsMacro[] = { { "USE_CBUFFER", "1" } };
    std::string vsPath = gShaderPathPrefix + "TestVS" + gShaderPathExt;
    mVertexShader.reset(CompileShader(vsPath.c_str(), ShaderType::Vertex, vsMacro, 1));
    if (!mVertexShader)
        return false;

    ShaderMacro psMacro[] = { { "USE_TEXTURE", "0" } };
    std::string psPath = gShaderPathPrefix + "TestPS" + gShaderPathExt;
    mPixelShader.reset(CompileShader(psPath.c_str(), ShaderType::Pixel, psMacro, 1));
    if (!mPixelShader)
        return false;

    ShaderProgramDesc shaderProgramDesc;
    shaderProgramDesc.vertexShader = mVertexShader.get();
    shaderProgramDesc.pixelShader = mPixelShader.get();
    mShaderProgram.reset(mRendererDevice->CreateShaderProgram(shaderProgramDesc));
    if (!mShaderProgram)
        return false;

    int cbufferSlot = mShaderProgram->GetResourceSlotByName("TestCBuffer");
    if (cbufferSlot < 0)
        return false;

    // create binding set for vertex shader bindings
    ResourceBindingDesc vertexShaderBinding(ShaderResourceType::CBuffer, cbufferSlot);
    mResBindingSet.reset(mRendererDevice->CreateResourceBindingSet(
        ResourceBindingSetDesc(&vertexShaderBinding, 1, ShaderType::Vertex)));
    if (!mResBindingSet)
        return false;

    // create binding layout
    IResourceBindingSet* bindingSet = mResBindingSet.get();
    mResBindingLayout.reset(mRendererDevice->CreateResourceBindingLayout(
        ResourceBindingLayoutDesc(&bindingSet, 1)));
    if (!mResBindingLayout)
        return false;

    DepthStateDesc depthStateDesc;
    depthStateDesc.depthCompareFunc = CompareFunc::Less;
    depthStateDesc.depthWriteEnable = true;
    depthStateDesc.depthTestEnable = true;
    depthStateDesc.stencilEnable = false;

    BlendStateDesc blendStateDesc;
    blendStateDesc.independent = false;
    blendStateDesc.rtDescs[0].enable = true;
    blendStateDesc.rtDescs[0].srcColorFunc = BlendFunc::SrcAlpha;
    blendStateDesc.rtDescs[0].destColorFunc = BlendFunc::OneMinusSrcAlpha;

    PipelineStateDesc psd;
    psd.raterizerState.cullMode = CullMode::Disabled;
    psd.primitiveType = PrimitiveType::Triangles;
    psd.vertexLayout = mVertexLayout.get();
    psd.resBindingLayout = mResBindingLayout.get();

    if (withStencil)
    {
        psd.depthState.depthWriteEnable = false;
        psd.depthState.depthTestEnable = false;
        psd.depthState.stencilOpPass = StencilOp::Replace;
        psd.depthState.stencilOpDepthFail = StencilOp::Replace;
        psd.depthState.stencilOpFail = StencilOp::Replace;
        psd.depthState.stencilFunc = CompareFunc::Pass;
        psd.depthState.stencilEnable = true;
        psd.depthState.stencilMask = 0xFF;
        mMaskPipelineState.reset(mRendererDevice->CreatePipelineState(psd));
        if (!mMaskPipelineState)
            return false;

        psd.depthState.depthCompareFunc = CompareFunc::Less;
        psd.depthState.depthWriteEnable = true;
        psd.depthState.depthTestEnable = true;
        psd.depthState.stencilOpPass = StencilOp::Keep;
        psd.depthState.stencilOpDepthFail = StencilOp::Keep;
        psd.depthState.stencilOpFail = StencilOp::Keep;
        psd.depthState.stencilFunc = CompareFunc::Equal;
        psd.depthState.stencilEnable = true;
        psd.depthState.stencilMask = 0xFF;
        mReflectionPipelineState.reset(mRendererDevice->CreatePipelineState(psd));
        if (!mReflectionPipelineState)
            return false;
    }

    psd.depthState = depthStateDesc;
    mCubePipelineState.reset(mRendererDevice->CreatePipelineState(psd));
    if (!mCubePipelineState)
        return false;

    psd.depthState = depthStateDesc;
    psd.blendState = blendStateDesc;
    mFloorPipelineState.reset(mRendererDevice->CreatePipelineState(psd));
    if (!mFloorPipelineState)
        return false;

    // create vertex buffers
    float vbData[] =
    {
        /// Vertex structure: pos.xyz, texCoord.uv, color.rgba

        /// cube
        -1.0f, -1.0f, -1.0f,  0.0f, 0.0f,  0.1f, 0.6f, 0.9f, 1.0f,
        -1.0f, -1.0f,  1.0f,  0.0f, 0.0f,  1.0f, 0.3f, 0.7f, 1.0f,
        -1.0f,  1.0f, -1.0f,  0.0f, 0.0f,  0.4f, 0.0f, 0.9f, 1.0f,
        -1.0f,  1.0f,  1.0f,  0.0f, 0.0f,  0.3f, 1.6f, 0.7f, 1.0f,
         1.0f, -1.0f, -1.0f,  0.0f, 0.0f,  0.1f, 0.7f, 0.2f, 1.0f,
         1.0f, -1.0f,  1.0f,  0.0f, 0.0f,  0.7f, 0.1f, 0.1f, 1.0f,
         1.0f,  1.0f, -1.0f,  0.0f, 0.0f,  0.8f, 0.3f, 0.5f, 1.0f,
         1.0f,  1.0f,  1.0f,  0.0f, 0.0f,  0.5f, 0.4f, 1.9f, 1.0f,

        /// plane (floor)
        -3.0f, -1.0f, -3.0f,  0.0f, 0.0f,  0.0f, 0.0f, 0.0f, 0.6f,
        -3.0f, -1.0f,  3.0f,  0.0f, 0.0f,  0.0f, 0.0f, 0.0f, 0.6f,
         3.0f, -1.0f, -3.0f,  0.0f, 0.0f,  0.0f, 0.0f, 0.0f, 0.6f,
         3.0f, -1.0f,  3.0f,  0.0f, 0.0f,  0.0f, 0.0f, 0.0f, 0.6f,
    };

    const uint16 ibData[] =
    {
        0, 1, 3,  0, 3, 2,
        1, 5, 7,  1, 7, 3,
        5, 6, 4,  5, 7, 6,
        4, 2, 0,  4, 6, 2,
        2, 3, 7,  2, 7, 6,
        4, 1, 0,  4, 5, 1,

        8, 9, 11,  8, 11, 10
    };

    BufferDesc bufferDesc;
    bufferDesc.type = BufferType::Vertex;
    bufferDesc.access = BufferAccess::GPU_ReadOnly;
    bufferDesc.size = sizeof(vbData);
    bufferDesc.initialData = vbData;
    mVertexBuffer.reset(mRendererDevice->CreateBuffer(bufferDesc));
    if (!mVertexBuffer)
        return false;

    bufferDesc.type = BufferType::Index;
    bufferDesc.access = BufferAccess::GPU_ReadOnly;
    bufferDesc.size = sizeof(ibData);
    bufferDesc.initialData = ibData;
    mIndexBuffer.reset(mRendererDevice->CreateBuffer(bufferDesc));
    if (!mIndexBuffer)
        return false;

    bufferDesc.type = BufferType::Constant;
    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = sizeof(VertexCBuffer);
    bufferDesc.initialData = nullptr;
    mConstantBuffer.reset(mRendererDevice->CreateBuffer(bufferDesc));
    if (!mConstantBuffer)
        return false;

    // create and fill binding set instance for cbuffer
    mResBindingInstance.reset(mRendererDevice->CreateResourceBindingInstance(mResBindingSet.get()));
    if (!mResBindingInstance)
        return false;
    if (!mResBindingInstance->WriteCBufferView(0, mConstantBuffer.get()))
        return false;

    return true;
}

bool DepthStencilScene::CreateDepthBuffer(bool withStencil)
{
    TextureDesc depthBufferDesc;
    depthBufferDesc.type = TextureType::Texture2D;
    depthBufferDesc.access = BufferAccess::GPU_ReadWrite;
    depthBufferDesc.width = static_cast<uint16>(WINDOW_WIDTH);
    depthBufferDesc.height = static_cast<uint16>(WINDOW_HEIGHT);
    depthBufferDesc.binding = NFE_RENDERER_TEXTURE_BIND_DEPTH;
    depthBufferDesc.mipmaps = 1;
    depthBufferDesc.depthBufferFormat = withStencil ?
        DepthBufferFormat::Depth24_Stencil8 : DepthBufferFormat::Depth16;
    depthBufferDesc.debugName = "DepthStencilScene::mDepthBuffer";
    mDepthBuffer.reset(mRendererDevice->CreateTexture(depthBufferDesc));
    if (!mDepthBuffer)
        return false;

    return true;
}

bool DepthStencilScene::CreateSubSceneNoDepthBuffer()
{
    return CreateBasicResources(false);
}

bool DepthStencilScene::CreateSubSceneDepthBuffer()
{
    if (!CreateDepthBuffer(false))
        return false;

    return CreateBasicResources(false);
}

bool DepthStencilScene::CreateSubSceneDepthStencilBuffer()
{
    if (!CreateDepthBuffer(true))
        return false;

    return CreateBasicResources(true);
}

bool DepthStencilScene::OnInit(void* winHandle)
{
    // create backbuffer connected with the window
    BackbufferDesc bbDesc;
    bbDesc.width = WINDOW_WIDTH;
    bbDesc.height = WINDOW_HEIGHT;
    bbDesc.windowHandle = winHandle;
    bbDesc.vSync = false;
    bbDesc.debugName = "DepthStencilScene::mWindowBackbuffer";
    mWindowBackbuffer.reset(mRendererDevice->CreateBackbuffer(bbDesc));
    if (!mWindowBackbuffer)
        return false;

    return true;
}

bool DepthStencilScene::OnSwitchSubscene()
{
    mAngle = 0.0f;
    return true;
}

void DepthStencilScene::Draw(float dt)
{
    mAngle += 2.0f * dt;
    if (mAngle > NFE_MATH_2PI)
        mAngle -= NFE_MATH_2PI;

    Matrix modelMatrix = MatrixRotationNormal(Vector(0.0f, 1.0f, 0.0f), mAngle);
    Matrix viewMatrix = MatrixLookTo(Vector(6.0f, 1.2f, 0.0f), Vector(-2.0f, -1.0f, 0.0f),
                                     Vector(0.0f, 1.0f, 0.0f));
    Matrix projMatrix = MatrixPerspective(static_cast<float>(WINDOW_WIDTH) /
                                          static_cast<float>(WINDOW_HEIGHT),
                                          70.0f * NFE_MATH_PI / 180.0f, 100.0f, 0.1f);

    Matrix reflectionMatrix = MatrixScaling(Vector(1.0f, -1.0f, 1.0f)) *
                              MatrixTranslation3(Vector(0.0f, -2.0f, 0.0f));

    mCommandBuffer->Reset();
    mCommandBuffer->SetViewport(0.0f, static_cast<float>(WINDOW_WIDTH), 0.0f,
                                static_cast<float>(WINDOW_HEIGHT), 0.0f, 1.0f);
    mCommandBuffer->SetRenderTarget(mWindowRenderTarget.get());
    mCommandBuffer->SetResourceBindingLayout(mResBindingLayout.get());
    mCommandBuffer->BindResources(0, mResBindingInstance.get());

    int stride = 9 * sizeof(float);
    int offset = 0;
    IBuffer* vb = mVertexBuffer.get();
    mCommandBuffer->SetVertexBuffers(1, &vb, &stride, &offset);
    mCommandBuffer->SetIndexBuffer(mIndexBuffer.get(), IndexBufferFormat::Uint16);
    mCommandBuffer->SetShaderProgram(mShaderProgram.get());

    IBuffer* cb = mConstantBuffer.get();
    VertexCBuffer cbuffer;

    if (GetCurrentSubSceneNumber() >= 2)
    {
        // clear depth-stencil buffer
        mCommandBuffer->Clear(NFE_CLEAR_FLAG_DEPTH | NFE_CLEAR_FLAG_STENCIL,
                              nullptr, 1.0f, 0);

        // set reflected matrix
        cbuffer.viewMatrix = modelMatrix * reflectionMatrix * viewMatrix * projMatrix;
        mCommandBuffer->WriteBuffer(cb, 0, sizeof(VertexCBuffer), &cbuffer);

        // Step 1: draw floor to stencil buffer
        mCommandBuffer->SetPipelineState(mMaskPipelineState.get());
        mCommandBuffer->SetStencilRef(0x01);
        mCommandBuffer->DrawIndexed(2 * 3, -1, 2 * 6 * 3);

        float color[] = { 0.7f, 0.8f, 0.9f, 1.0f };
        mCommandBuffer->Clear(NFE_CLEAR_FLAG_TARGET, color);

        // Step 2: draw cube reflection
        mCommandBuffer->SetPipelineState(mReflectionPipelineState.get());
        mCommandBuffer->DrawIndexed(2 * 6 * 3);
    }
    else
    {
        // clear depth buffer
        mCommandBuffer->Clear(NFE_CLEAR_FLAG_DEPTH, nullptr, 1.0f);

        float color[] = { 0.7f, 0.8f, 0.9f, 1.0f };
        mCommandBuffer->Clear(NFE_CLEAR_FLAG_TARGET, color);
    }

    // set "normal" matrix
    cbuffer.viewMatrix = modelMatrix * viewMatrix * projMatrix;
    mCommandBuffer->WriteBuffer(cb, 0, sizeof(VertexCBuffer), &cbuffer);

    // Step 3: draw floor
    mCommandBuffer->SetPipelineState(mFloorPipelineState.get());
    mCommandBuffer->DrawIndexed(2 * 3, -1, 2 * 6 * 3);

    // Step 4: draw "normal" cube
    mCommandBuffer->SetPipelineState(mCubePipelineState.get());
    mCommandBuffer->DrawIndexed(2 * 6 * 3);

    mRendererDevice->Execute(mCommandBuffer->Finish().get());
    mWindowBackbuffer->Present();
}

void DepthStencilScene::ReleaseSubsceneResources()
{
    mWindowRenderTarget.reset();
    mDepthBuffer.reset();
    mVertexShader.reset();
    mPixelShader.reset();
    mConstantBuffer.reset();
    mVertexBuffer.reset();
    mIndexBuffer.reset();
    mVertexLayout.reset();
    mShaderProgram.reset();
    mMaskPipelineState.reset();
    mReflectionPipelineState.reset();
    mFloorPipelineState.reset();
    mCubePipelineState.reset();

    mResBindingInstance.reset();
    mResBindingLayout.reset();
    mResBindingSet.reset();
}

void DepthStencilScene::Release()
{
    ReleaseSubsceneResources();
    mWindowBackbuffer.reset();
    mCommandBuffer = nullptr;
    mRendererDevice = nullptr;
}