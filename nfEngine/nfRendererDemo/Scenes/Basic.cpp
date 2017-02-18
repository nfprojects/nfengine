/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Source file with scene definitions
 */

#include "../PCH.hpp"

#include "Basic.hpp"
#include "../Common.hpp"

#include "nfCommon/Math/Math.hpp"
#include "nfCommon/Math/Matrix.hpp"
#include "nfCommon/Logger.hpp"

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

bool BasicScene::CreateShaders(bool useCBuffer, bool useTexture, BufferMode cbufferMode)
{
    mTextureSlot = -1;
    mCBufferSlot = -1;

    ShaderMacro vsMacro[] = { { "USE_CBUFFER", useCBuffer ? "1" : "0" } };
    std::string vsPath = gShaderPathPrefix + "TestVS" + gShaderPathExt;
    mVertexShader = CompileShader(vsPath.c_str(), ShaderType::Vertex, vsMacro, 1);
    if (!mVertexShader)
        return false;

    ShaderMacro psMacro[] = { { "USE_TEXTURE", useTexture ? "1" : "0" } };
    std::string psPath = gShaderPathPrefix + "TestPS" + gShaderPathExt;
    mPixelShader = CompileShader(psPath.c_str(), ShaderType::Pixel, psMacro, 1);
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

        if (cbufferMode == BufferMode::Static || cbufferMode == BufferMode::Dynamic)
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

    bool useVolatileCBufferBinding = useCBuffer && (cbufferMode == BufferMode::Volatile);
    VolatileCBufferBinding volatileCBufferBinding(ShaderType::Vertex,
                                                  ShaderResourceType::CBuffer,
                                                  mCBufferSlot);

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
    vbDesc.type = BufferType::Vertex;
    vbDesc.mode = BufferMode::Static;
    vbDesc.size = withExtraVert ? sizeof(vbDataExtra) : sizeof(vbData);
    vbDesc.initialData = withExtraVert ? vbDataExtra : vbData;
    mVertexBuffer = mRendererDevice->CreateBuffer(vbDesc);
    if (!mVertexBuffer)
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

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.rtFormats[0] = mBackbufferFormat;
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
    ibDesc.type = BufferType::Index;
    ibDesc.mode = BufferMode::Static;
    ibDesc.size = sizeof(ibData);
    ibDesc.initialData = ibData;
    mIndexBuffer = mRendererDevice->CreateBuffer(ibDesc);
    if (!mIndexBuffer)
        return false;

    return true;
}

bool BasicScene::CreateConstantBuffer(BufferMode cbufferMode)
{
    const Matrix rotMatrix = MatrixRotationNormal(Vector(0.0f, 0.0f, 1.0f), NFE_MATH_PI);
    mAngle = 0.0f;
    mCBufferMode = cbufferMode;

    BufferDesc cbufferDesc;
    cbufferDesc.type = BufferType::Constant;
    cbufferDesc.mode = cbufferMode;
    cbufferDesc.size = sizeof(VertexCBuffer);

    if (cbufferMode == BufferMode::Static)
    {
        cbufferDesc.initialData = &rotMatrix;
    }

    mConstantBuffer = mRendererDevice->CreateBuffer(cbufferDesc);
    if (!mConstantBuffer)
        return false;

    if (cbufferMode == BufferMode::Static || cbufferMode == BufferMode::Dynamic)
    {
        // create and fill binding set instance
        mVSBindingInstance = mRendererDevice->CreateResourceBindingInstance(mVSBindingSet);
        if (!mVSBindingInstance)
            return false;
        if (!mVSBindingInstance->WriteCBufferView(0, mConstantBuffer))
            return false;
    }

    return true;
}

bool BasicScene::CreateTexture()
{
    uint32_t bitmap[] = { 0xFFFFFFFF, 0, 0, 0xFFFFFFFF };
    TextureDataDesc textureDataDesc;
    textureDataDesc.data = bitmap;
    textureDataDesc.lineSize = 2 * sizeof(uint32_t);
    textureDataDesc.sliceSize = 4 * sizeof(uint32_t);

    TextureDesc textureDesc;
    textureDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER;
    textureDesc.format = ElementFormat::R8G8B8A8_U_Norm;
    textureDesc.width = 2;
    textureDesc.height = 2;
    textureDesc.mipmaps = 1;
    textureDesc.dataDesc = &textureDataDesc;
    textureDesc.layers = 1;
    mTexture = mRendererDevice->CreateTexture(textureDesc);
    if (!mTexture)
        return false;

    // create and fill binding set instance
    mPSBindingInstance = mRendererDevice->CreateResourceBindingInstance(mPSBindingSet);
    if (!mPSBindingInstance)
        return false;
    if (!mPSBindingInstance->WriteTextureView(0, mTexture))
        return false;

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

    return CreateShaders(false, false, BufferMode::Static);
}

// Adds vertex buffer creation
// Two colored triangles should be visible
bool BasicScene::CreateSubSceneVertexBuffer()
{
    mGridSize = 1;

    if (!CreateSampler())
        return false;

    if (!CreateShaders(false, false, BufferMode::Static))
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

    if (!CreateShaders(false, false, BufferMode::Static))
        return false;

    if (!CreateVertexBuffer(true))
        return false;

    return CreateIndexBuffer();
}

// Adds constant buffers
// The triangle and the square should rotate
bool BasicScene::CreateSubSceneConstantBuffer(BufferMode cbufferMode)
{
    mGridSize = 1;

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
bool BasicScene::CreateSubSceneTexture(BufferMode cbufferMode, int gridSize)
{
    mGridSize = gridSize;

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
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneConstantBuffer, this, BufferMode::Static), "ConstantBuffer (Static)");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneConstantBuffer, this, BufferMode::Dynamic), "ConstantBuffer (Dynamic)");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneConstantBuffer, this, BufferMode::Volatile), "ConstantBuffer (Volatile)");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneTexture, this, BufferMode::Dynamic, 1), "Texture + Dynamic CBuffer");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneTexture, this, BufferMode::Volatile, 1), "Texture + Volatile CBuffer");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneTexture, this, BufferMode::Volatile, 5), "CBufferStress5");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneTexture, this, BufferMode::Volatile, 30), "CBufferStress30");
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

void BasicScene::Draw(float dt)
{
    // reset bound resources and set them once again
    mCommandBuffer->Begin();
    mCommandBuffer->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);
    mCommandBuffer->SetScissors(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    mCommandBuffer->SetRenderTarget(mWindowRenderTarget);

    int stride = 9 * sizeof(float);
    int offset = 0;
    if (mIndexBuffer)
        mCommandBuffer->SetIndexBuffer(mIndexBuffer, IndexBufferFormat::Uint16);

    if (mVertexBuffer)
    {
        const BufferPtr& vb = mVertexBuffer;
        mCommandBuffer->SetVertexBuffers(1, &vb, &stride, &offset);
    }

    if (mResBindingLayout)
        mCommandBuffer->SetResourceBindingLayout(mResBindingLayout);

    if (mTexture)
        mCommandBuffer->BindResources(mPSBindingSlot, mPSBindingInstance);

    if (mConstantBuffer)
    {
        if (mCBufferMode == BufferMode::Static || mCBufferMode == BufferMode::Dynamic)
            mCommandBuffer->BindResources(mVSBindingSlot, mVSBindingInstance);
        else if (mCBufferMode == BufferMode::Volatile)
            mCommandBuffer->BindVolatileCBuffer(0, mConstantBuffer);
    }

    if (mPipelineState)
        mCommandBuffer->SetPipelineState(mPipelineState);

    // apply rotation
    mAngle += 2.0f * dt;
    if (mAngle > NFE_MATH_2PI)
        mAngle -= NFE_MATH_2PI;

    // clear target
    const Float4 color(0.0f, 0.0f, 0.0f, 1.0f);
    mCommandBuffer->Clear(ClearFlagsColor, 1, nullptr, &color);

    const float scaleCoeff = 1.0f / static_cast<float>(mGridSize);
    for (int i = 0; i < mGridSize; ++i)
    {
        for (int j = 0; j < mGridSize; ++j)
        {
            if (mConstantBuffer && (mCBufferMode == BufferMode::Dynamic || mCBufferMode == BufferMode::Volatile))
            {
                float xOffset = 2.0f * (static_cast<float>(i) + 0.5f) * scaleCoeff - 1.0f;
                float yOffset = 2.0f * (static_cast<float>(j) + 0.5f) * scaleCoeff - 1.0f;
                const float angle = mAngle + 5.0f * i + 7.0f * j;

                const Matrix rotMatrix = MatrixRotationNormal(Vector(0.0f, 0.0f, 1.0f), angle);
                const Matrix translationMatrix = MatrixTranslation3(Vector(xOffset, yOffset, 0.0f));
                const Matrix scaleMatrix = MatrixScaling(Vector(scaleCoeff, scaleCoeff, 0.0f));

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

    CommandListID commandList = mCommandBuffer->Finish();
    mRendererDevice->Execute(commandList);
    mWindowBackbuffer->Present();
    mRendererDevice->FinishFrame();
}

void BasicScene::Release()
{
    ReleaseSubsceneResources();
    mWindowRenderTarget.reset();
    mWindowBackbuffer.reset();
    mCommandBuffer = nullptr;
    mRendererDevice = nullptr;
}
