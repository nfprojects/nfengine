/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Source file with scene definitions
 */

#include "../PCH.hpp"

#include "Basic.hpp"
#include "../Common.hpp"

#include "../nfCommon/Math/Math.hpp"
#include "../nfCommon/Math/Matrix.hpp"

#include <vector>
#include <functional>

using namespace NFE::Math;
using namespace NFE::Common;
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

bool BasicScene::CreateShaderProgram(bool useCBuffer, bool useTexture)
{
    mTextureSlot = -1;
    mCBufferSlot = -1;

    ShaderMacro vsMacro[] = { { "USE_CBUFFER", useCBuffer ? "1" : "0" } };
    std::string vsPath = gShaderPathPrefix + "TestVS" + gShaderPathExt;
    mVertexShader.reset(CompileShader(vsPath.c_str(), ShaderType::Vertex, vsMacro, 1));
    if (!mVertexShader)
        return false;

    ShaderMacro psMacro[] = { { "USE_TEXTURE", useTexture ? "1" : "0" } };
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

    std::vector<IResourceBindingSet*> bindingSets;

    // create binding set
    if (useCBuffer)
    {
        mCBufferSlot = mShaderProgram->GetResourceSlotByName("TestCBuffer");
        if (mCBufferSlot < 0)
            return false;

        // create binding set for vertex shader bindings
        ResourceBindingDesc vertexShaderBinding(ShaderResourceType::CBuffer, mCBufferSlot);
        mVSBindingSet.reset(mRendererDevice->CreateResourceBindingSet(
            ResourceBindingSetDesc(&vertexShaderBinding, 1, ShaderType::Vertex)));
        if (!mVSBindingSet)
            return false;
        bindingSets.push_back(mVSBindingSet.get());

        if (useTexture)
        {
            mTextureSlot = mShaderProgram->GetResourceSlotByName("gTexture");
            if (mTextureSlot < 0)
                return false;

            // create binding set for pixel shader bindings
            ResourceBindingDesc pixelShaderBinding(ShaderResourceType::Texture, mTextureSlot);
            mPSBindingSet.reset(mRendererDevice->CreateResourceBindingSet(
                ResourceBindingSetDesc(&pixelShaderBinding, 1, ShaderType::Pixel)));
            if (!mPSBindingSet)
                return false;
            bindingSets.push_back(mPSBindingSet.get());
        }
    }

    // create binding layout
    mResBindingLayout.reset(mRendererDevice->CreateResourceBindingLayout(
        ResourceBindingLayoutDesc(bindingSets.data(), bindingSets.size())));
    if (!mResBindingLayout)
        return false;

    return true;
}

bool BasicScene::CreateVertexBuffer(bool withExtraVert)
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
    mVertexBuffer.reset(mRendererDevice->CreateBuffer(vbDesc));
    if (!mVertexBuffer)
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

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.blendState.independent = false;
    pipelineStateDesc.blendState.rtDescs[0].enable = true;
    pipelineStateDesc.vertexLayout = mVertexLayout.get();
    pipelineStateDesc.resBindingLayout = mResBindingLayout.get();
    mPipelineState.reset(mRendererDevice->CreatePipelineState(pipelineStateDesc));
    if (!mPipelineState)
        return false;

    return true;
}

bool BasicScene::CreateIndexBuffer()
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
    mIndexBuffer.reset(mRendererDevice->CreateBuffer(ibDesc));
    if (!mIndexBuffer)
        return false;

    return true;
}

bool BasicScene::CreateConstantBuffer()
{
    mAngle = 0.0f;

    BufferDesc vertexCBufferDesc;
    vertexCBufferDesc.type = BufferType::Constant;
    vertexCBufferDesc.access = BufferAccess::CPU_Write;
    vertexCBufferDesc.size = sizeof(VertexCBuffer);
    mConstantBuffer.reset(mRendererDevice->CreateBuffer(vertexCBufferDesc));
    if (!mConstantBuffer)
        return false;

    // create and fill binding set instance for cbuffer
    mVSBindingInstance.reset(mRendererDevice->CreateResourceBindingInstance(mVSBindingSet.get()));
    if (!mVSBindingInstance)
        return false;
    if (!mVSBindingInstance->WriteCBufferView(0, mConstantBuffer.get()))
        return false;

    return true;
}

bool BasicScene::CreateTexture()
{
    SamplerDesc samplerDesc;
    mSampler.reset(mRendererDevice->CreateSampler(samplerDesc));
    if (!mSampler)
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
    mTexture.reset(mRendererDevice->CreateTexture(textureDesc));
    if (!mTexture)
        return false;

    // create and fill binding set instance
    mPSBindingInstance.reset(mRendererDevice->CreateResourceBindingInstance(mPSBindingSet.get()));
    if (!mPSBindingInstance)
        return false;
    if (!mPSBindingInstance->WriteTextureView(0, mTexture.get(), mSampler.get()))
        return false;

    return true;
}


/////////////////
/// Subscenes ///
/////////////////

// Basic initialization, additionally to RT & BackBuffer shaders are compiled
// Empty window should be visible
bool BasicScene::CreateSubSceneEmpty()
{
    return CreateShaderProgram(false, false);
}

// Adds vertex buffer creation
// Two colored triangles should be visible
bool BasicScene::CreateSubSceneVertexBuffer()
{
    if (!CreateShaderProgram(false, false))
        return false;

    return CreateVertexBuffer(false);
}

// Adds IndexBuffer creation
// A colored triangle and a colored square should be visible
bool BasicScene::CreateSubSceneIndexBuffer()
{
    if (!CreateShaderProgram(false, false))
        return false;

    if (!CreateVertexBuffer(true))
        return false;

    return CreateIndexBuffer();
}

// Adds constant buffers
// The triangle and the square should rotate
bool BasicScene::CreateSubSceneConstantBuffer()
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
bool BasicScene::CreateSubSceneTexture()
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


/////////////////////////////////////////////////
/// BasicScene methods and virtuals overriden ///
/////////////////////////////////////////////////

BasicScene::BasicScene()
    : Scene("Basic")
{
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneEmpty, this), "Empty");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneVertexBuffer, this), "VertexBuffer");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneIndexBuffer, this), "IndexBuffer");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneConstantBuffer, this), "ConstantBuffer");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneTexture, this), "Texture");
}

BasicScene::~BasicScene()
{
    Release();
}

void BasicScene::ReleaseSubsceneResources()
{
    // clear resources
    mTexture.reset();
    mSampler.reset();
    mConstantBuffer.reset();
    mIndexBuffer.reset();
    mVertexLayout.reset();
    mVertexBuffer.reset();
    mPixelShader.reset();
    mVertexShader.reset();
    mPipelineState.reset();
    mShaderProgram.reset();

    mVSBindingInstance.reset();
    mPSBindingInstance.reset();
    mResBindingLayout.reset();
    mVSBindingSet.reset();
    mPSBindingSet.reset();
}

bool BasicScene::OnInit(void* winHandle)
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

    mCommandBuffer->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);

    return true;
}

bool BasicScene::OnSwitchSubscene()
{
    return true;
}

void BasicScene::Draw(float dt)
{
    // reset bound resources and set them once again
    mCommandBuffer->Reset();
    mCommandBuffer->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);

    if (mPipelineState)
        mCommandBuffer->SetPipelineState(mPipelineState.get());

    mCommandBuffer->SetRenderTarget(mWindowRenderTarget.get());

    if (mShaderProgram)
        mCommandBuffer->SetShaderProgram(mShaderProgram.get());

    int stride = 9 * sizeof(float);
    int offset = 0;
    if (mIndexBuffer)
        mCommandBuffer->SetIndexBuffer(mIndexBuffer.get(), IndexBufferFormat::Uint16);

    if (mVertexBuffer)
    {
        IBuffer* vb = mVertexBuffer.get();
        mCommandBuffer->SetVertexBuffers(1, &vb, &stride, &offset);
    }

    if (mConstantBuffer)
        mCommandBuffer->BindResources(0, mVSBindingInstance.get());

    if (mTexture)
        mCommandBuffer->BindResources(1, mPSBindingInstance.get());

    // apply rotation
    mAngle += 2.0f * dt;
    if (mAngle > NFE_MATH_2PI)
        mAngle -= NFE_MATH_2PI;

    if (mConstantBuffer)
    {
        VertexCBuffer vertexCBufferData;
        vertexCBufferData.viewMatrix = MatrixRotationNormal(Vector(0.0f, 0.0f, 1.0f), mAngle);
        mCommandBuffer->WriteBuffer(mConstantBuffer.get(), 0, sizeof(VertexCBuffer),
                                    &vertexCBufferData);
    }

    // clear target
    float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    mCommandBuffer->Clear(NFE_CLEAR_FLAG_TARGET, color);

    // draw
    if (mIndexBuffer)
        mCommandBuffer->DrawIndexed(PrimitiveType::Triangles, 9);
    else if (mVertexBuffer)
        mCommandBuffer->Draw(PrimitiveType::Triangles, 6);

    ICommandList* commandList = mCommandBuffer->Finish();
    mRendererDevice->Execute(commandList);
    delete commandList;

    mWindowBackbuffer->Present();
}

void BasicScene::Release()
{
    ReleaseSubsceneResources();
    mWindowRenderTarget.reset();
    mWindowBackbuffer.reset();
    mCommandBuffer = nullptr;
    mRendererDevice = nullptr;
}
