/**
 * @file
 * @author Lookey (costyrra.xl@gmail.com)
 * @brief  Source file with scene definitions
 */

#include "../PCH.hpp"

#include "Scene.hpp"

#include "Engine/Common/Math/Math.hpp"
#include "Engine/Common/Math/Matrix4.hpp"
#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Renderers/RendererCommon/Fence.hpp"
#include "Engine/Common/Reflection/ReflectionClassDefine.hpp"


class BasicScene : public Scene
{
    NFE_DECLARE_POLYMORPHIC_CLASS(BasicScene)

    // Renderer interfaces generated by BasicScene
    NFE::Renderer::RenderTargetPtr mWindowRenderTarget;
    NFE::Renderer::PipelineStatePtr mPipelineState;
    NFE::Renderer::ShaderPtr mVertexShader;
    NFE::Renderer::ShaderPtr mPixelShader;
    NFE::Renderer::BufferPtr mVertexBuffer;
    NFE::Renderer::VertexLayoutPtr mVertexLayout;
    NFE::Renderer::BufferPtr mIndexBuffer;
    NFE::Renderer::BufferPtr mConstantBuffer;
    NFE::Renderer::SamplerPtr mSampler;
    NFE::Renderer::TexturePtr mTexture;

    int mCBufferSlot;
    int mTextureSlot;
    int mSamplerSlot;

    // Used for objects rotation in Constant Buffer scenes and onward
    float mAngle;

    // how many instances will be drawn?
    int mGridSize;

    // bind resources via binding instance or directly
    bool mUseBindingInstance;

    // cbuffer mode
    NFE::Renderer::ResourceAccessMode mCBufferMode;

    // Releases only subscene-related resources. Backbuffer, RT and BlendState stay intact.
    void ReleaseSubsceneResources() override;

    // Resource creators for subscenes
    bool CreateShaders(bool useCBuffer, bool useTexture);
    bool CreateVertexBuffer(bool withExtraVert);
    bool CreateIndexBuffer();
    bool CreatePipelineState(NFE::Renderer::ResourceAccessMode cbufferMode);
    bool CreateConstantBuffer(NFE::Renderer::ResourceAccessMode cbufferMode);
    bool CreateTexture();
    bool CreateSampler();

    // Subscenes
    bool CreateSubSceneEmpty();
    bool CreateSubSceneVertexBuffer();
    bool CreateSubSceneIndexBuffer();
    bool CreateSubSceneConstantBuffer(NFE::Renderer::ResourceAccessMode cbufferMode);
    bool CreateSubSceneTexture(NFE::Renderer::ResourceAccessMode cbufferMode, int gridSize = 1);

public:
    BasicScene();
    ~BasicScene();

    bool OnInit(void* winHandle) override;
    void Draw(float dt) override;
    void Release() override;
};


NFE_DEFINE_POLYMORPHIC_CLASS(BasicScene)
    NFE_CLASS_PARENT(Scene)
NFE_END_DEFINE_CLASS()


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

bool BasicScene::CreateShaders(bool useCBuffer, bool useTexture)
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

            mSamplerSlot = mPixelShader->GetResourceSlotByName("gSampler");
            if (mSamplerSlot < 0)
                return false;
        }
    }

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
    vbDesc.usage = BufferUsageFlag::VertexBuffer;
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
    ibDesc.usage = BufferUsageFlag::IndexBuffer;
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

bool BasicScene::CreatePipelineState(ResourceAccessMode cbufferMode)
{
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

    VolatileBufferBinding vbBinding;
    vbBinding.stage = ShaderType::Vertex;
    vbBinding.binding = mCBufferSlot;

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.renderTargetFormats = { Format::R8G8B8A8_U_Norm };
    pipelineStateDesc.vertexShader = mVertexShader;
    pipelineStateDesc.pixelShader = mPixelShader;
    pipelineStateDesc.blendState.independent = false;
    pipelineStateDesc.blendState.rtDescs[0].enable = true;
    pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
    pipelineStateDesc.vertexLayout = mVertexLayout;
    if (cbufferMode == ResourceAccessMode::Volatile)
    {
        pipelineStateDesc.volatileBufferBindingCount = 1;
        pipelineStateDesc.volatileBufferBindings = &vbBinding;
    }
    mPipelineState = mRendererDevice->CreatePipelineState(pipelineStateDesc);
    if (!mPipelineState)
        return false;

    return true;
}

bool BasicScene::CreateConstantBuffer(ResourceAccessMode cbufferMode)
{
    mAngle = 0.0f;
    mCBufferMode = cbufferMode;

    BufferDesc cbufferDesc;
    cbufferDesc.mode = cbufferMode;
    cbufferDesc.usage = BufferUsageFlag::ConstantBuffer;
    cbufferDesc.size = sizeof(VertexCBuffer);
    cbufferDesc.debugName = "BasicScene::CreateConstantBuffer";

    mConstantBuffer = mRendererDevice->CreateBuffer(cbufferDesc);
    if (!mConstantBuffer)
        return false;

    return true;
}

bool BasicScene::CreateTexture()
{
    TextureDesc textureDesc;
    textureDesc.usage = TextureUsageFlag::ReadonlyShaderResource;
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

    return CreateShaders(false, false);
}

// Adds vertex buffer creation
// Two colored triangles should be visible
bool BasicScene::CreateSubSceneVertexBuffer()
{
    mGridSize = 1;

    if (!CreateSampler())
        return false;

    if (!CreateShaders(false, false))
        return false;

    if (!CreateVertexBuffer(false))
        return false;

    return CreatePipelineState(ResourceAccessMode::Invalid);
}

// Adds IndexBuffer creation
// A colored triangle and a colored square should be visible
bool BasicScene::CreateSubSceneIndexBuffer()
{
    mGridSize = 1;

    if (!CreateSampler())
        return false;

    if (!CreateShaders(false, false))
        return false;

    if (!CreateVertexBuffer(true))
        return false;

    if (!CreateIndexBuffer())
        return false;

    return CreatePipelineState(ResourceAccessMode::Invalid);
}

// Adds constant buffers
// The triangle and the square should rotate
bool BasicScene::CreateSubSceneConstantBuffer(ResourceAccessMode cbufferMode)
{
    mGridSize = 1;

    if (!CreateSampler())
        return false;

    if (!CreateShaders(true, false))
        return false;

    if (!CreateVertexBuffer(true))
        return false;

    if (!CreateIndexBuffer())
        return false;

    if (!CreateConstantBuffer(cbufferMode))
        return false;

    return CreatePipelineState(cbufferMode);
}

// Add texture support
// The triangle should be rendered checked
bool BasicScene::CreateSubSceneTexture(ResourceAccessMode cbufferMode, int gridSize)
{
    mGridSize = gridSize;

    if (!CreateSampler())
        return false;

    if (!CreateShaders(true, true))
        return false;

    if (!CreateVertexBuffer(true))
        return false;

    if (!CreateIndexBuffer())
        return false;

    if (!CreateConstantBuffer(cbufferMode))
        return false;

    if (!CreatePipelineState(cbufferMode))
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
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneConstantBuffer, this, ResourceAccessMode::GPUOnly), "ConstantBuffer (GPUOnly)");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneConstantBuffer, this, ResourceAccessMode::Volatile), "ConstantBuffer (Volatile)");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneTexture, this, ResourceAccessMode::GPUOnly, 1), "Texture + CBuffer");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneTexture, this, ResourceAccessMode::Volatile, 1), "Texture + Volatile CBuffer");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneTexture, this, ResourceAccessMode::Volatile, 5), "CBufferStress5");
    RegisterSubScene(std::bind(&BasicScene::CreateSubSceneTexture, this, ResourceAccessMode::Volatile, 30), "CBufferStress30");
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
}

bool BasicScene::OnInit(void* winHandle)
{
    if (!Scene::OnInit(winHandle))
    {
        return false;
    }

    // create rendertarget that will render to the window's backbuffer
    RenderTargetDesc rtDesc;
    rtDesc.targets = { RenderTargetElement(mWindowRenderTargetTexture) };
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

    if (mTexture)
    {
        mCommandBuffer->BindTexture(ShaderType::Pixel, mTextureSlot, mTexture);
        mCommandBuffer->BindSampler(ShaderType::Pixel, mSamplerSlot, mSampler);
    }

    if (mConstantBuffer)
        mCommandBuffer->BindConstantBuffer(ShaderType::Vertex, mCBufferSlot, mConstantBuffer);

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
