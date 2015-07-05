/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Source file with scene definitions
 */

#include "PCH.hpp"

#include "BasicScene.hpp"
#include "Common.hpp"

#include "../nfCommon/Math/Math.hpp"

#include <vector>
#include <functional>

using namespace NFE::Math;
using namespace NFE::Common;

std::string gUsedBackend;

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

IShader* BasicScene::CompileShader(const char* path, ShaderType type, ShaderMacro* macros, size_t macrosNum)
{
    ShaderDesc desc;
    desc.type = type;
    desc.path = path;
    desc.macros = macros;
    desc.macrosNum = macrosNum;
    return mRendererDevice->CreateShader(desc);
}

/// Helper creators for the Scene

bool BasicScene::CreateShaderProgram(bool useCBuffer, bool useTexture)
{
    ShaderMacro vsMacro[] = { { "USE_CBUFFER", useCBuffer ? "1" : "0" } };
    std::string vsPath = mShaderPathPrefix + "TestVS" + mShaderPathExt;
    mVertexShader.reset(CompileShader(vsPath.c_str(), ShaderType::Vertex, vsMacro, 1));
    if (!mVertexShader)
        return false;

    ShaderMacro psMacro[] = { { "USE_TEXTURE", useTexture ? "1" : "0" } };
    std::string psPath = mShaderPathPrefix + "TestPS" + mShaderPathExt;
    mPixelShader.reset(CompileShader(psPath.c_str(), ShaderType::Pixel, psMacro, 1));
    if (!mPixelShader)
        return false;

    mShaderProgramDesc.vertexShader = mVertexShader.get();
    mShaderProgramDesc.pixelShader = mPixelShader.get();
    mShaderProgram.reset(mRendererDevice->CreateShaderProgram(mShaderProgramDesc));
    if (!mShaderProgram)
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
    vertexLayoutDesc.vertexShader = mShaderProgramDesc.vertexShader;
    mVertexLayout.reset(mRendererDevice->CreateVertexLayout(vertexLayoutDesc));
    if (!mVertexLayout)
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

BasicScene::BasicScene(const std::string& shaderPathPrefix, const std::string& shaderPathExt)
    : mShaderPathPrefix(shaderPathPrefix)
    , mShaderPathExt(shaderPathExt)
    , mAngle(0.0f)
    , mHighestAvailableSubScene(0)
{
    mSubScenes.push_back(std::bind(&BasicScene::CreateSubSceneEmpty, this));
    mSubScenes.push_back(std::bind(&BasicScene::CreateSubSceneVertexBuffer, this));
    mSubScenes.push_back(std::bind(&BasicScene::CreateSubSceneIndexBuffer, this));
    mSubScenes.push_back(std::bind(&BasicScene::CreateSubSceneConstantBuffer, this));
    mSubScenes.push_back(std::bind(&BasicScene::CreateSubSceneTexture, this));
}

BasicScene::~BasicScene()
{
    Release();
}

void BasicScene::ReleaseSubsceneResources()
{
    mTexture.reset();
    mSampler.reset();

    // cleanup after cbuffer
    if (mConstantBuffer)
    {
        IBuffer* emptyCB = nullptr;
        mCommandBuffer->SetConstantBuffers(&emptyCB, 1, ShaderType::Vertex);
    }
    mConstantBuffer.reset();

    mIndexBuffer.reset();
    mVertexLayout.reset();
    mVertexBuffer.reset();
    mPixelShader.reset();
    mVertexShader.reset();
    mShaderProgram.reset();
}

bool BasicScene::Init(IDevice* rendererDevice, void* winHandle)
{
    mRendererDevice = rendererDevice;

    mCommandBuffer = mRendererDevice->GetDefaultCommandBuffer();

    // create backbuffer connected with the window
    BackbufferDesc bbDesc;
    bbDesc.width = WINDOW_WIDTH;
    bbDesc.height = WINDOW_HEIGHT;
    bbDesc.windowHandle = winHandle;
    bbDesc.vSync = true;
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

    BlendStateDesc blendStateDesc;
    blendStateDesc.independent = false;
    blendStateDesc.rtDescs[0].enable = true;
    mBlendState.reset(mRendererDevice->CreateBlendState(blendStateDesc));
    if (!mBlendState)
        return false;

    mCommandBuffer->SetBlendState(mBlendState.get());

    /// Basic stuff initialized, try to find the highest subscene possible
    for (mHighestAvailableSubScene = mSubScenes.size()-1; ; mHighestAvailableSubScene--)
    {
        if (mSubScenes[mHighestAvailableSubScene]())
            break; // the scene initialized successfully

        if (mHighestAvailableSubScene == 0)
            return false; // we hit the end of our scenes vector, no scene successfully inited

        // nope, release it and continue checking
        ReleaseSubsceneResources();
    }

    return true;
}

bool BasicScene::SwitchSubScene(size_t subScene)
{
    ReleaseSubsceneResources();
    return mSubScenes[subScene]();
}

size_t BasicScene::GetAvailableSubSceneCount()
{
    return mHighestAvailableSubScene;
}

void BasicScene::Draw()
{
    mAngle += 0.03f;
    if (mAngle > NFE_MATH_2PI)
        mAngle -= NFE_MATH_2PI;

    if (mConstantBuffer)
    {
        VertexCBuffer vertexCBufferData;
        vertexCBufferData.viewMatrix = MatrixRotationNormal(Vector(0.0f, 0.0f, 1.0f), mAngle);
        mCommandBuffer->WriteBuffer(mConstantBuffer.get(), 0, sizeof(VertexCBuffer), &vertexCBufferData);
    }

    float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    mCommandBuffer->SetRenderTarget(mWindowRenderTarget.get());
    mCommandBuffer->Clear(NFE_CLEAR_FLAG_TARGET, color);
    if (mShaderProgram) mCommandBuffer->SetShaderProgram(mShaderProgram.get());
    if (mVertexLayout) mCommandBuffer->SetVertexLayout(mVertexLayout.get());

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
    {
        IBuffer* cb = mConstantBuffer.get();
        mCommandBuffer->SetConstantBuffers(&cb, 1, ShaderType::Vertex);
    }

    if (mTexture)
    {
        ITexture* tex = mTexture.get();
        mCommandBuffer->SetTextures(&tex, 1, ShaderType::Pixel);
    }

    if (mSampler)
    {
        ISampler* sampler = mSampler.get();
        mCommandBuffer->SetSamplers(&sampler, 1, ShaderType::Pixel);
    }

    if (mIndexBuffer)
        mCommandBuffer->DrawIndexed(PrimitiveType::Triangles, 9);
    else if (mVertexBuffer)
        mCommandBuffer->Draw(PrimitiveType::Triangles, 6);

    mWindowBackbuffer->Present();
}

void BasicScene::Release()
{
    ReleaseSubsceneResources();
    mBlendState.reset();
    mWindowRenderTarget.reset();
    mWindowBackbuffer.reset();
    mCommandBuffer = nullptr;
    mRendererDevice = nullptr;
}
