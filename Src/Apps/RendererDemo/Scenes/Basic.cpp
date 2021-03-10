/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Source file with scene definitions
 */

#include "../PCH.hpp"

#include "Basic.hpp"
#include "../Common.hpp"

#include "Engine/Common/Math/Math.hpp"
#include "Engine/Common/Math/Matrix4.hpp"
#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Renderers/RendererCommon/Fence.hpp"

#include <vector>
#include <functional>


using namespace NFE;
using namespace NFE::Math;
using namespace NFE::Renderer;

namespace {

struct VertexCBuffer
{
    Matrix4 viewMatrix;
};

struct PixelCBuffer
{
    Matrix4 viewMatrix;
};

} // namespace

/// Helper creators for the Scene

bool BasicScene::CreateShaders(bool useCBuffer, bool useTexture, ResourceAccessMode cbufferMode)
{
    mTextureSlot = -1;
    mCBufferSlot = -1;

    ShaderMacro vsMacro[] = { { "USE_CBUFFER", useCBuffer ? "1" : "0" } };
    const Common::String vsPath = gShaderPathPrefix + "TestVS" + gShaderPathExt;
    mVertexShader = CompileShader(vsPath.Str(), ShaderType::Vertex, vsMacro, 1);
    if (!mVertexShader)
        return false;

    ShaderMacro psMacro[] = { { "USE_TEXTURE", useTexture ? "1" : "0" } };
    const Common::String psPath = gShaderPathPrefix + "TestPS" + gShaderPathExt;
    mPixelShader = CompileShader(psPath.Str(), ShaderType::Pixel, psMacro, 1);
    if (!mPixelShader)
        return false;

    int numBindingSets = 0;
    ResourceBindingSetPtr bindingSets[2];

    // create binding set
    if (useCBuffer)
    {
        mCBufferSlot = mVertexShader->GetResourceSlotByName("TestCBuffer");
        if (mCBufferSlot < 0)
            return false;

        if (useTexture)
        {
            mTextureSlot = mPixelShader->GetResourceSlotByName("gTexture");
            if (mTextureSlot < 0)
                return false;

            // create binding set for pixel shader bindings
            ResourceBindingDesc pixelShaderBinding(ShaderResourceType::Texture,
                                                   mTextureSlot,
                                                   mSampler);
            mPSBindingSet = mRendererDevice->CreateResourceBindingSet(ResourceBindingSetDesc(&pixelShaderBinding, 1, ShaderType::Pixel));
            if (!mPSBindingSet)
                return false;

            mPSBindingSlot = numBindingSets++;
            bindingSets[mPSBindingSlot] = mPSBindingSet;
        }

        if (cbufferMode == ResourceAccessMode::GPUOnly)
        {
            ResourceBindingDesc vertexShaderBinding(ShaderResourceType::CBuffer,
                                                    mCBufferSlot,
                                                    mSampler);
            mVSBindingSet = mRendererDevice->CreateResourceBindingSet(ResourceBindingSetDesc(&vertexShaderBinding, 1, ShaderType::Vertex));
            if (!mVSBindingSet)
                return false;

            mVSBindingSlot = numBindingSets++;
            bindingSets[mVSBindingSlot] = mVSBindingSet;
        }
    }

    bool useVolatileCBufferBinding = useCBuffer && (cbufferMode == ResourceAccessMode::Volatile);
    VolatileCBufferBinding volatileCBufferBinding(ShaderType::Vertex,
                                                  ShaderResourceType::CBuffer,
                                                  mCBufferSlot, sizeof(VertexCBuffer));

    // create binding layout
    mResBindingLayout = mRendererDevice->CreateResourceBindingLayout(
        ResourceBindingLayoutDesc(bindingSets, numBindingSets, &volatileCBufferBinding, useVolatileCBufferBinding ? 1 : 0));
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
    vbDesc.size = withExtraVert ? sizeof(vbDataExtra) : sizeof(vbData);
    vbDesc.usage = NFE_RENDERER_BUFFER_USAGE_VERTEX_BUFFER;
    mVertexBuffer = mRendererDevice->CreateBuffer(vbDesc);
    if (!mVertexBuffer)
        return false;

    // upload vertex buffer data
    {
        mCommandBuffer->Begin(CommandQueueType::Copy);
        mCommandBuffer->WriteBuffer(mVertexBuffer, 0, vbDesc.size, withExtraVert ? vbDataExtra : vbData);
        mCopyQueue->Execute(mCommandBuffer->Finish());
        mCopyQueue->Signal()->Wait();
    }

    VertexLayoutElement vertexLayoutElements[] =
    {
        { Format::R32G32B32_Float,       0, 0, false, 0 }, // position
        { Format::R32G32_Float,          12, 0, false, 0 }, // tex-coords
        { Format::R32G32B32A32_Float,    20, 0, false, 0 }, // color
    };

    VertexLayoutDesc vertexLayoutDesc;
    vertexLayoutDesc.elements = vertexLayoutElements;
    vertexLayoutDesc.numElements = 3;
    mVertexLayout = mRendererDevice->CreateVertexLayout(vertexLayoutDesc);
    if (!mVertexLayout)
        return false;

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.rtFormats[0] = Format::R8G8B8A8_U_Norm;
    pipelineStateDesc.vertexShader = mVertexShader;
    pipelineStateDesc.pixelShader = mPixelShader;
    pipelineStateDesc.blendState.independent = false;
    pipelineStateDesc.blendState.rtDescs[0].enable = true;
    pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
    pipelineStateDesc.vertexLayout = mVertexLayout;
    pipelineStateDesc.resBindingLayout = mResBindingLayout;
    mPipelineState = mRendererDevice->CreatePipelineState(pipelineStateDesc);
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
    ibDesc.size = sizeof(ibData);
    ibDesc.usage = NFE_RENDERER_BUFFER_USAGE_INDEX_BUFFER;
    mIndexBuffer = mRendererDevice->CreateBuffer(ibDesc);
    if (!mIndexBuffer)
        return false;

    // upload index buffer data
    {
        mCommandBuffer->Begin(CommandQueueType::Copy);
        mCommandBuffer->WriteBuffer(mIndexBuffer, 0, ibDesc.size, ibData);
        mCopyQueue->Execute(mCommandBuffer->Finish());
        mCopyQueue->Signal()->Wait();
    }

    return true;
}

bool BasicScene::CreateConstantBuffer(ResourceAccessMode cbufferMode)
{
    mAngle = 0.0f;
    mCBufferMode = cbufferMode;

    BufferDesc cbufferDesc;
    cbufferDesc.mode = cbufferMode;
    cbufferDesc.usage = NFE_RENDERER_BUFFER_USAGE_CONSTANT_BUFFER;
    cbufferDesc.size = sizeof(VertexCBuffer);
    cbufferDesc.debugName = "BasicScene::CreateConstantBuffer";

    mConstantBuffer = mRendererDevice->CreateBuffer(cbufferDesc);
    if (!mConstantBuffer)
        return false;

    if (cbufferMode == ResourceAccessMode::GPUOnly)
    {
        // create and fill binding set instance
        mVSBindingInstance = mRendererDevice->CreateResourceBindingInstance(mVSBindingSet);
        if (!mVSBindingInstance)
            return false;
        if (!mVSBindingInstance->SetCBufferView(0, mConstantBuffer))
            return false;
        if (!mVSBindingInstance->Finalize())
            return false;
    }

    return true;
}

bool BasicScene::CreateTexture()
{
    TextureDesc textureDesc;
    textureDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER;
    textureDesc.format = Format::R8G8B8A8_U_Norm;
    textureDesc.width = 2;
    textureDesc.height = 2;
    textureDesc.mipmaps = 1;
    textureDesc.layers = 1;
    textureDesc.debugName = "BasicScene::CreateTexture";
    mTexture = mRendererDevice->CreateTexture(textureDesc);
    if (!mTexture)
        return false;

    // upload texture data
    {
        const uint32 bitmap[] = { 0xFFFFFFFF, 0, 0, 0xFFFFFFFF };

        mCommandBuffer->Begin(CommandQueueType::Copy);
        mCommandBuffer->WriteTexture(mTexture, bitmap);
        mCopyQueue->Execute(mCommandBuffer->Finish());
        mCopyQueue->Signal()->Wait();
    }

    if (mUseBindingInstance)
    {
        // create and fill binding set instance
        mPSBindingInstance = mRendererDevice->CreateResourceBindingInstance(mPSBindingSet);
        if (!mPSBindingInstance)
            return false;
        if (!mPSBindingInstance->SetTextureView(0, mTexture))
            return false;
        if (!mPSBindingInstance->Finalize())
            return false;
    }

    return true;
}

bool BasicScene::CreateSampler()
{
    SamplerDesc samplerDesc;
    mSampler = mRendererDevice->CreateSampler(samplerDesc);
    return mSampler != nullptr;
}


/////////////////
/// Subscenes ///
/////////////////

// Basic initialization, additionally to RT & BackBuffer shaders are compiled
// Empty window should be visible
bool BasicScene::CreateSubSceneEmpty()
{
    mGridSize = 1;

    if (!CreateSampler())
        return false;

    return CreateShaders(false, false, ResourceAccessMode::GPUOnly);
}

// Adds vertex buffer creation
// Two colored triangles should be visible
bool BasicScene::CreateSubSceneVertexBuffer()
{
    mGridSize = 1;

    if (!CreateSampler())
        return false;

    if (!CreateShaders(false, false, ResourceAccessMode::GPUOnly))
        return false;

    return CreateVertexBuffer(false);
}

// Adds IndexBuffer creation
// A colored triangle and a colored square should be visible
bool BasicScene::CreateSubSceneIndexBuffer()
{
    mGridSize = 1;

    if (!CreateSampler())
        return false;

    if (!CreateShaders(false, false, ResourceAccessMode::GPUOnly))
        return false;

    if (!CreateVertexBuffer(true))
        return false;

    return CreateIndexBuffer();
}

// Adds constant buffers
// The triangle and the square should rotate
bool BasicScene::CreateSubSceneConstantBuffer(ResourceAccessMode cbufferMode, bool useBindingInstance)
{
    mGridSize = 1;
    mUseBindingInstance = useBindingInstance;

    if (!CreateSampler())
        return false;

    if (!CreateShaders(true, false, cbufferMode))
        return false;

    if (!CreateVertexBuffer(true))
        return false;

    if (!CreateIndexBuffer())
        return false;

    return CreateConstantBuffer(cbufferMode);
}

// Add texture support
// The triangle should be rendered checked
bool BasicScene::CreateSubSceneTexture(ResourceAccessMode cbufferMode, bool useBindingInstance, int gridSize)
{
    mGridSize = gridSize;
    mUseBindingInstance = useBindingInstance;

    if (!CreateSampler())
        return false;

    if (!CreateShaders(true, true, cbufferMode))
        return false;

    if (!CreateVertexBuffer(true))
        return false;

    if (!CreateIndexBuffer())
        return false;

    if (!CreateConstantBuffer(cbufferMode))
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
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneConstantBuffer, this, ResourceAccessMode::GPUOnly, true), "ConstantBuffer (Dynamic)");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneConstantBuffer, this, ResourceAccessMode::Volatile, true), "ConstantBuffer (Volatile)");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneTexture, this, ResourceAccessMode::GPUOnly, true, 1), "Texture + Dynamic CBuffer");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneTexture, this, ResourceAccessMode::GPUOnly, false, 1), "Texture + Dynamic CBuffer + Direct Binding");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneTexture, this, ResourceAccessMode::Volatile, true, 1), "Texture + Volatile CBuffer");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneTexture, this, ResourceAccessMode::Volatile, true, 5), "CBufferStress5");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneTexture, this, ResourceAccessMode::Volatile, false, 30), "CBufferStress30 + Direct Binding");
}

BasicScene::~BasicScene()
{
    Release();
}

void BasicScene::ReleaseSubsceneResources()
{
    Scene::ReleaseSubsceneResources();

    // clear resources
    mTexture.Reset();
    mSampler.Reset();
    mConstantBuffer.Reset();
    mIndexBuffer.Reset();
    mVertexLayout.Reset();
    mVertexBuffer.Reset();
    mPixelShader.Reset();
    mVertexShader.Reset();
    mPipelineState.Reset();

    mVSBindingInstance.Reset();
    mPSBindingInstance.Reset();
    mResBindingLayout.Reset();
    mVSBindingSet.Reset();
    mPSBindingSet.Reset();
}

bool BasicScene::OnInit(void* winHandle)
{
    if (!Scene::OnInit(winHandle))
    {
        return false;
    }

    // create rendertarget that will render to the window's backbuffer
    RenderTargetElement rtTarget;
    rtTarget.texture = mWindowRenderTargetTexture;
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;
    mWindowRenderTarget = mRendererDevice->CreateRenderTarget(rtDesc);
    if (!mWindowRenderTarget)
        return false;

    return true;
}

void BasicScene::Draw(float dt)
{
    // reset bound resources and set them once again
    mCommandBuffer->Begin(CommandQueueType::Graphics);

    mCommandBuffer->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);
    mCommandBuffer->SetScissors(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    mCommandBuffer->SetRenderTarget(mWindowRenderTarget);

    uint32 stride = 9 * sizeof(float);
    uint32 offset = 0;
    if (mIndexBuffer)
        mCommandBuffer->SetIndexBuffer(mIndexBuffer, IndexBufferFormat::Uint16);

    if (mVertexBuffer)
    {
        const BufferPtr& vb = mVertexBuffer;
        mCommandBuffer->SetVertexBuffers(1, &vb, &stride, &offset);
    }

    if (mResBindingLayout)
        mCommandBuffer->SetResourceBindingLayout(PipelineType::Graphics, mResBindingLayout);

    if (mTexture)
    {
        if (mUseBindingInstance)
        {
            mCommandBuffer->BindResources(PipelineType::Graphics, mPSBindingSlot, mPSBindingInstance);
        }
        else
        {
            mCommandBuffer->BindTexture(PipelineType::Graphics, mPSBindingSlot, 0, mTexture);
        }
    }

    if (mConstantBuffer)
    {
        if (mCBufferMode == ResourceAccessMode::GPUOnly)
            mCommandBuffer->BindResources(PipelineType::Graphics, mVSBindingSlot, mVSBindingInstance);
        else if (mCBufferMode == ResourceAccessMode::Volatile)
            mCommandBuffer->BindVolatileCBuffer(PipelineType::Graphics, 0, mConstantBuffer);
    }

    if (mPipelineState)
        mCommandBuffer->SetPipelineState(mPipelineState);

    // apply rotation
    mAngle += 2.0f * dt;
    if (mAngle > 2.0f * Constants::pi<float>)
        mAngle -= 2.0f * Constants::pi<float>;

    // clear target
    const Vec4fU color(0.0f, 0.0f, 0.0f, 1.0f);
    mCommandBuffer->Clear(ClearFlagsColor, 1, nullptr, &color);

    const float scaleCoeff = 1.0f / static_cast<float>(mGridSize);
    for (int i = 0; i < mGridSize; ++i)
    {
        for (int j = 0; j < mGridSize; ++j)
        {
            if (mConstantBuffer && (mCBufferMode == ResourceAccessMode::GPUOnly || mCBufferMode == ResourceAccessMode::Volatile))
            {
                float xOffset = 2.0f * (static_cast<float>(i) + 0.5f) * scaleCoeff - 1.0f;
                float yOffset = 2.0f * (static_cast<float>(j) + 0.5f) * scaleCoeff - 1.0f;
                const float angle = mAngle + 5.0f * i + 7.0f * j;

                const Matrix4 rotMatrix = Matrix4::MakeRotationNormal(Vec4f(0.0f, 0.0f, 1.0f), angle);
                const Matrix4 translationMatrix = Matrix4::MakeTranslation(Vec4f(xOffset, yOffset, 0.0f));
                const Matrix4 scaleMatrix = Matrix4::MakeScaling(Vec4f(scaleCoeff, scaleCoeff, 0.0f));

                VertexCBuffer vertexCBufferData;
                vertexCBufferData.viewMatrix = scaleMatrix * rotMatrix * translationMatrix;
                mCommandBuffer->WriteBuffer(mConstantBuffer, 0, sizeof(VertexCBuffer), &vertexCBufferData);
            }

            // draw
            if (mIndexBuffer)
                mCommandBuffer->DrawIndexed(9, 1);
            else if (mVertexBuffer)
                mCommandBuffer->Draw(6, 1);
        }
    }

    mCommandBuffer->CopyTexture(mWindowRenderTargetTexture, mWindowBackbuffer);
    CommandListPtr commandList = mCommandBuffer->Finish();
    mGraphicsQueue->Execute(commandList);

    mWindowBackbuffer->Present();
    mRendererDevice->FinishFrame();
}

void BasicScene::Release()
{
    ReleaseSubsceneResources();
    mWindowRenderTarget.Reset();
    mWindowBackbuffer.Reset();
    mCommandBuffer.Reset();
    mRendererDevice = nullptr;
}
