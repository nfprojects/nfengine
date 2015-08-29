/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  DepthStencil scene definition
 */

#include "PCH.hpp"

#include "DepthStencilScene.hpp"
#include "Common.hpp"

#include "../nfCommon/Math/Math.hpp"

#include <vector>
#include <functional>

using namespace NFE::Math;
using namespace NFE::Common;

namespace {

struct VertexCBuffer
{
    Matrix viewMatrix;
};

} // namespace


DepthStencilScene::DepthStencilScene(const std::string& shaderPathPrefix,
                                     const std::string& shaderPathExt)
    : mShaderPathPrefix(shaderPathPrefix)
    , mShaderPathExt(shaderPathExt)
{
    // mSubScenes.push_back(std::bind(&BasicScene::CreateSubSceneEmpty, this));
}

DepthStencilScene::~DepthStencilScene()
{
    Release();
}

IShader* DepthStencilScene::CompileShader(const char* path, ShaderType type,
                                          ShaderMacro* macros, size_t macrosNum)
{
    ShaderDesc desc;
    desc.type = type;
    desc.path = path;
    desc.macros = macros;
    desc.macrosNum = macrosNum;
    return mRendererDevice->CreateShader(desc);
}

bool DepthStencilScene::Init(IDevice* rendererDevice, void* winHandle)
{
    mRendererDevice = rendererDevice;

    mCommandBuffer = mRendererDevice->GetDefaultCommandBuffer();

    // create backbuffer connected with the window
    BackbufferDesc bbDesc;
    bbDesc.width = WINDOW_WIDTH;
    bbDesc.height = WINDOW_HEIGHT;
    bbDesc.windowHandle = winHandle;
    bbDesc.vSync = true;
    bbDesc.debugName = "DepthStencilScene::mWindowBackbuffer";
    mWindowBackbuffer.reset(mRendererDevice->CreateBackbuffer(bbDesc));
    if (!mWindowBackbuffer)
        return false;

    TextureDesc depthBufferDesc;
    depthBufferDesc.type = TextureType::Texture2D;
    depthBufferDesc.access = BufferAccess::GPU_ReadWrite;
    depthBufferDesc.width = WINDOW_WIDTH;
    depthBufferDesc.height = WINDOW_HEIGHT;
    depthBufferDesc.binding = NFE_RENDERER_TEXTURE_BIND_DEPTH;
    depthBufferDesc.mipmaps = 1;
    depthBufferDesc.depthBufferFormat = DepthBufferFormat::Depth24_Stencil8;
    depthBufferDesc.debugName = "DepthStencilScene::mDepthBuffer";
    mDepthBuffer.reset(mRendererDevice->CreateTexture(depthBufferDesc));
    if (!mDepthBuffer)
        return false;

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

    DepthStateDesc depthStateDesc;
    depthStateDesc.depthWriteEnable = false;
    depthStateDesc.depthTestEnable = false;
    depthStateDesc.stencilOpPass = StencilOperation::Replace;
    depthStateDesc.stencilOpDepthFail = StencilOperation::Replace;
    depthStateDesc.stencilOpFail = StencilOperation::Replace;
    depthStateDesc.stencilFunc = CompareFunc::Pass;
    depthStateDesc.stencilEnable = true;
    depthStateDesc.stencilReadMask = 0x0;
    depthStateDesc.stencilWriteMask = 0xFF;
    mMaskDepthState.reset(mRendererDevice->CreateDepthState(depthStateDesc));
    if (!mMaskDepthState)
        return false;

    depthStateDesc.depthCompareFunc = CompareFunc::Less;
    depthStateDesc.depthWriteEnable = true;
    depthStateDesc.depthTestEnable = true;
    depthStateDesc.stencilOpPass = StencilOperation::Keep;
    depthStateDesc.stencilOpDepthFail = StencilOperation::Keep;
    depthStateDesc.stencilOpFail = StencilOperation::Keep;
    depthStateDesc.stencilFunc = CompareFunc::Equal;
    depthStateDesc.stencilEnable = true;
    depthStateDesc.stencilReadMask = 0xFF;
    depthStateDesc.stencilWriteMask = 0x0;
    mReflectionDepthState.reset(mRendererDevice->CreateDepthState(depthStateDesc));
    if (!mReflectionDepthState)
        return false;

    depthStateDesc.stencilEnable = false;
    mDepthState.reset(mRendererDevice->CreateDepthState(depthStateDesc));
    if (!mDepthState)
        return false;

    RasterizerStateDesc rasterizerStateDesc;
    rasterizerStateDesc.cullMode = CullMode::Disabled;
    mRasterizerState.reset(mRendererDevice->CreateRasterizerState(rasterizerStateDesc));
    if (!mRasterizerState)
        return false;

    BlendStateDesc blendStateDesc;
    blendStateDesc.independent = false;
    blendStateDesc.rtDescs[0].enable = true;
    blendStateDesc.rtDescs[0].srcColorFunc = BlendFunc::SrcAlpha;
    blendStateDesc.rtDescs[0].destColorFunc = BlendFunc::OneMinusSrcAlpha;
    mFloorBlendState.reset(mRendererDevice->CreateBlendState(blendStateDesc));
    if (!mFloorBlendState)
        return false;

    ShaderMacro vsMacro[] = { { "USE_CBUFFER", "1" } };
    std::string vsPath = mShaderPathPrefix + "TestVS" + mShaderPathExt;
    mVertexShader.reset(CompileShader(vsPath.c_str(), ShaderType::Vertex, vsMacro, 1));
    if (!mVertexShader)
        return false;

    ShaderMacro psMacro[] = { { "USE_TEXTURE", "0" } };
    std::string psPath = mShaderPathPrefix + "TestPS" + mShaderPathExt;
    mPixelShader.reset(CompileShader(psPath.c_str(), ShaderType::Pixel, psMacro, 1));
    if (!mPixelShader)
        return false;

    mShaderProgramDesc.vertexShader = mVertexShader.get();
    mShaderProgramDesc.pixelShader = mPixelShader.get();
    mShaderProgram.reset(mRendererDevice->CreateShaderProgram(mShaderProgramDesc));
    if (!mShaderProgram)
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

    VertexLayoutElement vertexLayoutElements[] =
    {
        { ElementFormat::Float_32, 3,  0, 0, false, 0 }, // position
        { ElementFormat::Float_32, 2, 12, 0, false, 0 }, // tex-coords
        { ElementFormat::Float_32, 4, 20, 0, false, 0 }, // color
    };

    VertexLayoutDesc vertexLayoutDesc;
    vertexLayoutDesc.elements = vertexLayoutElements;
    vertexLayoutDesc.numElements = 3;
    vertexLayoutDesc.vertexShader = mShaderProgramDesc.vertexShader;
    mVertexLayout.reset(mRendererDevice->CreateVertexLayout(vertexLayoutDesc));
    if (!mVertexLayout)
        return false;

    bufferDesc.type = BufferType::Constant;
    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = sizeof(VertexCBuffer);
    bufferDesc.initialData = nullptr;
    mConstantBuffer.reset(mRendererDevice->CreateBuffer(bufferDesc));
    if (!mConstantBuffer)
        return false;

    SwitchSubscene(0);
    return true;
}

bool DepthStencilScene::SwitchSubscene(size_t subScene)
{
    (void)subScene;

    mAngle = 0.0f;

    // mCommandBuffer->Reset();
    // mCommandBuffer->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);
    // mCommandBuffer->SetRenderTarget(mWindowRenderTarget.get());

    return true;
}

size_t DepthStencilScene::GetAvailableSubSceneCount()
{
    return 1;
}

void DepthStencilScene::Draw()
{
    mAngle += 0.06f;

    Matrix modelMatrix = MatrixRotationNormal(Vector(0.0f, 1.0f, 0.0f), mAngle);
    Matrix viewMatrix = MatrixLookTo(Vector(6.0f, 1.2f, 0.0f), Vector(-2.0f, -1.0f, 0.0f),
                                     Vector(0.0f, 1.0f, 0.0f));
    Matrix projMatrix = MatrixPerspective((float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
                                          70.0f * NFE_MATH_PI / 180.0f, 100.0f, 0.1f);

    Matrix reflectionMatrix = MatrixScaling(Vector(1.0f, -1.0f, 1.0f)) *
                              MatrixTranslation3(Vector(0.0f, -2.0f, 0.0f));

    VertexCBuffer cbuffer;

    mCommandBuffer->Reset();
    mCommandBuffer->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);
    mCommandBuffer->SetRenderTarget(mWindowRenderTarget.get());

    // clear target
    mCommandBuffer->Clear(NFE_CLEAR_FLAG_DEPTH | NFE_CLEAR_FLAG_STENCIL,
                          nullptr, 1.0f, 0);

    int stride = 9 * sizeof(float);
    int offset = 0;
    IBuffer* vb = mVertexBuffer.get();
    mCommandBuffer->SetVertexBuffers(1, &vb, &stride, &offset);
    mCommandBuffer->SetIndexBuffer(mIndexBuffer.get(), IndexBufferFormat::Uint16);

    IBuffer* cb = mConstantBuffer.get();
    mCommandBuffer->SetConstantBuffers(&cb, 1, ShaderType::Vertex);

    mCommandBuffer->SetShaderProgram(mShaderProgram.get());
    mCommandBuffer->SetVertexLayout(mVertexLayout.get());
    mCommandBuffer->SetRasterizerState(mRasterizerState.get());

    // set reflected matrix
    cbuffer.viewMatrix = modelMatrix * reflectionMatrix * viewMatrix * projMatrix;
    mCommandBuffer->WriteBuffer(cb, 0, sizeof(VertexCBuffer), &cbuffer);

    // Step 1: draw floor to stencil buffer
    mCommandBuffer->SetBlendState(nullptr);
    mCommandBuffer->SetDepthState(mMaskDepthState.get());
    mCommandBuffer->SetStencilRef(0x01);
    mCommandBuffer->DrawIndexed(PrimitiveType::Triangles, 2 * 3, -1, 2 * 6 * 3);

    float color[] = { 0.7f, 0.8f, 0.9f, 1.0f };
    mCommandBuffer->Clear(NFE_CLEAR_FLAG_TARGET, color);

    // Step 2: draw cube reflection
    mCommandBuffer->SetDepthState(mReflectionDepthState.get());
    
    mCommandBuffer->DrawIndexed(PrimitiveType::Triangles, 2 * 6 * 3);

    // set normal matrix
    cbuffer.viewMatrix = modelMatrix * viewMatrix * projMatrix;
    mCommandBuffer->WriteBuffer(cb, 0, sizeof(VertexCBuffer), &cbuffer);

    // Step 3: draw floor
    mCommandBuffer->SetDepthState(mDepthState.get());
    mCommandBuffer->SetBlendState(mFloorBlendState.get());
    mCommandBuffer->DrawIndexed(PrimitiveType::Triangles, 2 * 3, -1, 2 * 6 * 3);

    // Step 4: draw "normal" cube
    mCommandBuffer->SetBlendState(nullptr);
    mCommandBuffer->DrawIndexed(PrimitiveType::Triangles, 2 * 6 * 3);

    mWindowBackbuffer->Present();
}

void DepthStencilScene::Release()
{
    mWindowRenderTarget.reset();
    mWindowBackbuffer.reset();

    // TODO

    mCommandBuffer = nullptr;
    mRendererDevice = nullptr;
}
