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

    std::vector<IResourceBindingSet*> bindingSets;

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
                                                   mSampler.get());
            mPSBindingSet.reset(mRendererDevice->CreateResourceBindingSet(
                ResourceBindingSetDesc(&pixelShaderBinding, 1, ShaderType::Pixel)));
            if (!mPSBindingSet)
                return false;
            bindingSets.push_back(mPSBindingSet.get());
        }
    }

    DynamicBufferBindingDesc dynBufferBindingDesc(ShaderType::Vertex,
                                                  ShaderResourceType::CBuffer,
                                                  mCBufferSlot);

    // create binding layout
    mResBindingLayout.reset(mRendererDevice->CreateResourceBindingLayout(
        ResourceBindingLayoutDesc(bindingSets.data(), bindingSets.size(),
                                  &dynBufferBindingDesc, useCBuffer ? 1 : 0)));
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
    mVertexBuffer.reset(mRendererDevice->CreateBuffer(vbDesc));
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
    mVertexLayout.reset(mRendererDevice->CreateVertexLayout(vertexLayoutDesc));
    if (!mVertexLayout)
        return false;

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.vertexShader = mVertexShader.get();
    pipelineStateDesc.pixelShader = mPixelShader.get();
    pipelineStateDesc.blendState.independent = false;
    pipelineStateDesc.blendState.rtDescs[0].enable = true;
    pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
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
    ibDesc.mode = BufferMode::Static;
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
    vertexCBufferDesc.mode = BufferMode::Volatile;
    vertexCBufferDesc.size = sizeof(VertexCBuffer);
    mConstantBuffer.reset(mRendererDevice->CreateBuffer(vertexCBufferDesc));
    if (!mConstantBuffer)
        return false;

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
    mTexture.reset(mRendererDevice->CreateTexture(textureDesc));
    if (!mTexture)
        return false;

    // create and fill binding set instance
    mPSBindingInstance.reset(mRendererDevice->CreateResourceBindingInstance(mPSBindingSet.get()));
    if (!mPSBindingInstance)
        return false;
    if (!mPSBindingInstance->WriteTextureView(0, mTexture.get()))
        return false;

    return true;
}

bool BasicScene::CreateSampler()
{
    SamplerDesc samplerDesc;
    mSampler.reset(mRendererDevice->CreateSampler(samplerDesc));
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

    return CreateShaderProgram(false, false);
}

// Adds vertex buffer creation
// Two colored triangles should be visible
bool BasicScene::CreateSubSceneVertexBuffer()
{
    mGridSize = 1;

    if (!CreateSampler())
        return false;

    if (!CreateShaderProgram(false, false))
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
    mGridSize = 1;

    if (!CreateSampler())
        return false;

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
bool BasicScene::CreateSubSceneTexture(int gridSize)
{
    mGridSize = gridSize;

    if (!CreateSampler())
        return false;

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
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneTexture, this, 1), "Texture");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneTexture, this, 5), "CBufferStress5");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneTexture, this, 30), "CBufferStress30");
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

    return true;
}

void BasicScene::Draw(float dt)
{
    // reset bound resources and set them once again
    mCommandBuffer->Reset();
    mCommandBuffer->SetViewport(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, 0.0f, 1.0f);
    mCommandBuffer->SetScissors(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    mCommandBuffer->SetRenderTarget(mWindowRenderTarget.get());

    int stride = 9 * sizeof(float);
    int offset = 0;
    if (mIndexBuffer)
        mCommandBuffer->SetIndexBuffer(mIndexBuffer.get(), IndexBufferFormat::Uint16);

    if (mVertexBuffer)
    {
        IBuffer* vb = mVertexBuffer.get();
        mCommandBuffer->SetVertexBuffers(1, &vb, &stride, &offset);
    }

    if (mResBindingLayout)
        mCommandBuffer->SetResourceBindingLayout(mResBindingLayout.get());

    if (mConstantBuffer)
        mCommandBuffer->BindDynamicBuffer(0, mConstantBuffer.get());

    if (mTexture)
        mCommandBuffer->BindResources(0, mPSBindingInstance.get());

    if (mPipelineState)
        mCommandBuffer->SetPipelineState(mPipelineState.get());

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
            if (mConstantBuffer)
            {
                float xOffset = 2.0f * (static_cast<float>(i) + 0.5f) * scaleCoeff - 1.0f;
                float yOffset = 2.0f * (static_cast<float>(j) + 0.5f) * scaleCoeff - 1.0f;
                const float angle = mAngle + 5.0f * i + 7.0f * j;

                const Matrix rotMatrix = MatrixRotationNormal(Vector(0.0f, 0.0f, 1.0f), angle);
                const Matrix translationMatrix = MatrixTranslation3(Vector(xOffset, yOffset, 0.0f));
                const Matrix scaleMatrix = MatrixScaling(Vector(scaleCoeff, scaleCoeff, 0.0f));

                VertexCBuffer vertexCBufferData;
                vertexCBufferData.viewMatrix = scaleMatrix * rotMatrix * translationMatrix;
                mCommandBuffer->WriteBuffer(mConstantBuffer.get(), 0, sizeof(VertexCBuffer),
                                            &vertexCBufferData);
            }

            // draw
            if (mIndexBuffer)
                mCommandBuffer->DrawIndexed(9, 1);
            else if (mVertexBuffer)
                mCommandBuffer->Draw(6, 1);
        }
    }

    mRendererDevice->Execute(mCommandBuffer->Finish().get());
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
