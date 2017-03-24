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

bool DepthStencilScene::CreateBasicResources(bool withDepth, bool withStencil)
{
    // create rendertarget that will render to the window's backbuffer
    RenderTargetElement rtTarget;
    rtTarget.texture = mWindowBackbuffer;
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;
    rtDesc.depthBuffer = mDepthBuffer;
    rtDesc.debugName = "DepthStencilScene::mWindowRenderTarget";
    mWindowRenderTarget = mRendererDevice->CreateRenderTarget(rtDesc);
    if (!mWindowRenderTarget)
        return false;

    VertexLayoutElement vertexLayoutElements[] =
    {
        { ElementFormat::R32G32B32_Float,       0, 0, false, 0 }, // position
        { ElementFormat::R32G32_Float,          12, 0, false, 0 }, // tex-coords
        { ElementFormat::R32G32B32A32_Float,    20, 0, false, 0 }, // color
    };

    VertexLayoutDesc vertexLayoutDesc;
    vertexLayoutDesc.elements = vertexLayoutElements;
    vertexLayoutDesc.numElements = 3;
    mVertexLayout = mRendererDevice->CreateVertexLayout(vertexLayoutDesc);
    if (!mVertexLayout)
        return false;

    ShaderMacro vsMacro[] = { { "USE_CBUFFER", "1" } };
    std::string vsPath = gShaderPathPrefix + "TestVS" + gShaderPathExt;
    mVertexShader = CompileShader(vsPath.c_str(), ShaderType::Vertex, vsMacro, 1);
    if (!mVertexShader)
        return false;

    ShaderMacro psMacro[] = { { "USE_TEXTURE", "0" } };
    std::string psPath = gShaderPathPrefix + "TestPS" + gShaderPathExt;
    mPixelShader = CompileShader(psPath.c_str(), ShaderType::Pixel, psMacro, 1);
    if (!mPixelShader)
        return false;

    int cbufferSlot = mVertexShader->GetResourceSlotByName("TestCBuffer");
    if (cbufferSlot < 0)
        return false;

    // create binding layout
    VolatileCBufferBinding cbufferDesc(ShaderType::Vertex, ShaderResourceType::CBuffer, 0,
                                       sizeof(VertexCBuffer));
    mResBindingLayout = mRendererDevice->CreateResourceBindingLayout(ResourceBindingLayoutDesc(nullptr, 0, &cbufferDesc, 1));
    if (!mResBindingLayout)
        return false;

    DepthStateDesc depthStateDesc;
    if (withDepth)
    {
        depthStateDesc.depthCompareFunc = CompareFunc::Less;
        depthStateDesc.depthWriteEnable = true;
        depthStateDesc.depthTestEnable = true;
        depthStateDesc.stencilEnable = false;
    }

    BlendStateDesc blendStateDesc;
    blendStateDesc.independent = false;
    blendStateDesc.rtDescs[0].enable = true;
    blendStateDesc.rtDescs[0].srcColorFunc = BlendFunc::SrcAlpha;
    blendStateDesc.rtDescs[0].destColorFunc = BlendFunc::OneMinusSrcAlpha;

    PipelineStateDesc psd;
    psd.rtFormats[0] = mBackbufferFormat;
    psd.vertexShader = mVertexShader;
    psd.pixelShader = mPixelShader;
    psd.raterizerState.cullMode = CullMode::Disabled;
    psd.primitiveType = PrimitiveType::Triangles;
    psd.vertexLayout = mVertexLayout;
    psd.resBindingLayout = mResBindingLayout;

    if (withDepth && !withStencil)
        psd.depthFormat = DepthBufferFormat::Depth16;
    else if (withDepth)
    {
        psd.depthFormat = DepthBufferFormat::Depth24_Stencil8;
        psd.depthState.depthWriteEnable = false;
        psd.depthState.depthTestEnable = false;
        psd.depthState.stencilOpPass = StencilOp::Replace;
        psd.depthState.stencilOpDepthFail = StencilOp::Replace;
        psd.depthState.stencilOpFail = StencilOp::Replace;
        psd.depthState.stencilFunc = CompareFunc::Pass;
        psd.depthState.stencilEnable = true;
        psd.depthState.stencilMask = 0xFF;
        mMaskPipelineState = mRendererDevice->CreatePipelineState(psd);
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
        mReflectionPipelineState = mRendererDevice->CreatePipelineState(psd);
        if (!mReflectionPipelineState)
            return false;
    }

    psd.depthState = depthStateDesc;
    mCubePipelineState = mRendererDevice->CreatePipelineState(psd);
    if (!mCubePipelineState)
        return false;

    psd.depthState = depthStateDesc;
    psd.blendState = blendStateDesc;
    mFloorPipelineState = mRendererDevice->CreatePipelineState(psd);
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
    bufferDesc.mode = BufferMode::Static;
    bufferDesc.size = sizeof(vbData);
    bufferDesc.initialData = vbData;
    mVertexBuffer = mRendererDevice->CreateBuffer(bufferDesc);
    if (!mVertexBuffer)
        return false;

    bufferDesc.type = BufferType::Index;
    bufferDesc.mode = BufferMode::Static;
    bufferDesc.size = sizeof(ibData);
    bufferDesc.initialData = ibData;
    mIndexBuffer = mRendererDevice->CreateBuffer(bufferDesc);
    if (!mIndexBuffer)
        return false;

    bufferDesc.type = BufferType::Constant;
    bufferDesc.mode = BufferMode::Volatile;
    bufferDesc.size = sizeof(VertexCBuffer);
    bufferDesc.initialData = nullptr;
    mConstantBuffer = mRendererDevice->CreateBuffer(bufferDesc);
    if (!mConstantBuffer)
        return false;

    return true;
}

bool DepthStencilScene::CreateDepthBuffer(bool withStencil)
{
    TextureDesc depthBufferDesc;
    depthBufferDesc.type = TextureType::Texture2D;
    depthBufferDesc.mode = BufferMode::GPUOnly;
    depthBufferDesc.width = static_cast<uint16>(WINDOW_WIDTH);
    depthBufferDesc.height = static_cast<uint16>(WINDOW_HEIGHT);
    depthBufferDesc.binding = NFE_RENDERER_TEXTURE_BIND_DEPTH;
    depthBufferDesc.mipmaps = 1;
    depthBufferDesc.depthBufferFormat = withStencil ?
        DepthBufferFormat::Depth24_Stencil8 : DepthBufferFormat::Depth16;
    depthBufferDesc.debugName = "DepthStencilScene::mDepthBuffer";
    mDepthBuffer = mRendererDevice->CreateTexture(depthBufferDesc);
    if (!mDepthBuffer)
        return false;

    return true;
}

bool DepthStencilScene::CreateSubSceneNoDepthBuffer()
{
    return CreateBasicResources(false, false);
}

bool DepthStencilScene::CreateSubSceneDepthBuffer()
{
    if (!CreateDepthBuffer(false))
        return false;

    return CreateBasicResources(true, false);
}

bool DepthStencilScene::CreateSubSceneDepthStencilBuffer()
{
    if (!CreateDepthBuffer(true))
        return false;

    return CreateBasicResources(true, true);
}

bool DepthStencilScene::OnInit(void* winHandle)
{
    // create backbuffer connected with the window
    BackbufferDesc bbDesc;
    bbDesc.width = WINDOW_WIDTH;
    bbDesc.height = WINDOW_HEIGHT;
    bbDesc.format = mBackbufferFormat;
    bbDesc.windowHandle = winHandle;
    bbDesc.vSync = false;
    bbDesc.debugName = "DepthStencilScene::mWindowBackbuffer";
    mWindowBackbuffer = mRendererDevice->CreateBackbuffer(bbDesc);
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

    Matrix modelMatrix = Matrix::MakeRotationNormal(Vector(0.0f, 1.0f, 0.0f), mAngle);
    Matrix viewMatrix = Matrix::MakeLookTo(Vector(6.0f, 1.2f, 0.0f), Vector(-2.0f, -1.0f, 0.0f),
                                     Vector(0.0f, 1.0f, 0.0f));
    Matrix projMatrix = Matrix::MakePerspective(static_cast<float>(WINDOW_WIDTH) /
                                          static_cast<float>(WINDOW_HEIGHT),
                                          70.0f * NFE_MATH_PI / 180.0f, 100.0f, 0.1f);

    Matrix reflectionMatrix = Matrix::MakeScaling(Vector(1.0f, -1.0f, 1.0f)) *
                              Matrix::MakeTranslation3(Vector(0.0f, -2.0f, 0.0f));

    mCommandBuffer->Begin();
    mCommandBuffer->SetViewport(0.0f, static_cast<float>(WINDOW_WIDTH), 0.0f,
                                static_cast<float>(WINDOW_HEIGHT), 0.0f, 1.0f);
    mCommandBuffer->SetScissors(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    mCommandBuffer->SetRenderTarget(mWindowRenderTarget);
    mCommandBuffer->SetResourceBindingLayout(mResBindingLayout);
    mCommandBuffer->BindVolatileCBuffer(0, mConstantBuffer);

    int stride = 9 * sizeof(float);
    int offset = 0;
    BufferPtr vb = mVertexBuffer;
    mCommandBuffer->SetVertexBuffers(1, &vb, &stride, &offset);
    mCommandBuffer->SetIndexBuffer(mIndexBuffer, IndexBufferFormat::Uint16);

    BufferPtr cb = mConstantBuffer;
    VertexCBuffer cbuffer;

    if (GetCurrentSubSceneNumber() >= 2)
    {
        // clear depth-stencil buffer
        mCommandBuffer->Clear(ClearFlagsDepth | ClearFlagsStencil, 0, nullptr, nullptr, 1.0f, 0);

        // set reflected matrix
        cbuffer.viewMatrix = modelMatrix * reflectionMatrix * viewMatrix * projMatrix;
        mCommandBuffer->WriteBuffer(cb, 0, sizeof(VertexCBuffer), &cbuffer);

        // Step 1: draw floor to stencil buffer
        mCommandBuffer->SetPipelineState(mMaskPipelineState);
        mCommandBuffer->SetStencilRef(0x01);
        mCommandBuffer->DrawIndexed(2 * 3, 1, 2 * 6 * 3);

        const Float4 color(0.7f, 0.8f, 0.9f, 1.0f);
        mCommandBuffer->Clear(ClearFlagsColor, 1, nullptr, &color);

        // Step 2: draw cube reflection
        mCommandBuffer->SetPipelineState(mReflectionPipelineState);
        mCommandBuffer->DrawIndexed(2 * 6 * 3);
    }
    else
    {
        // clear depth buffer
        mCommandBuffer->Clear(ClearFlagsDepth, 0, nullptr, nullptr, 1.0f);

        const Float4 color(0.7f, 0.8f, 0.9f, 1.0f);
        mCommandBuffer->Clear(ClearFlagsColor, 1, nullptr, &color);
    }

    // set "normal" matrix
    cbuffer.viewMatrix = modelMatrix * viewMatrix * projMatrix;
    mCommandBuffer->WriteBuffer(cb, 0, sizeof(VertexCBuffer), &cbuffer);

    // Step 3: draw floor
    mCommandBuffer->SetPipelineState(mFloorPipelineState);
    mCommandBuffer->DrawIndexed(2 * 3, 1, 2 * 6 * 3);

    // Step 4: draw "normal" cube
    mCommandBuffer->SetPipelineState(mCubePipelineState);
    mCommandBuffer->DrawIndexed(2 * 6 * 3);

    CommandListID commandList = mCommandBuffer->Finish();
    mRendererDevice->Execute(commandList);
    mWindowBackbuffer->Present();
    mRendererDevice->FinishFrame();
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
    mMaskPipelineState.reset();
    mReflectionPipelineState.reset();
    mFloorPipelineState.reset();
    mCubePipelineState.reset();
    mResBindingLayout.reset();
}

void DepthStencilScene::Release()
{
    ReleaseSubsceneResources();
    mWindowBackbuffer.reset();
    mCommandBuffer = nullptr;
    mRendererDevice = nullptr;
}
