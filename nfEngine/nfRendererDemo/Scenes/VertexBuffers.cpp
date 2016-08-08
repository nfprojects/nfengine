/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  VertexBuffersScene definition
 */

#include "../PCH.hpp"

#include "VertexBuffers.hpp"
#include "../Common.hpp"

#include "nfCommon/Math/Vector.hpp"
#include "nfCommon/Math/Random.hpp"

#include <vector>
#include <functional>


using namespace NFE;
using namespace NFE::Math;
using namespace NFE::Renderer;

namespace {

struct InstanceData
{
    Float3 pos;
    Float4 color;
};

int gInstancesNumber = 200;

} // namespace


bool VertexBuffersScene::CreateShaderProgram(bool useInstancing)
{
    ShaderMacro vsMacro[] = { { "USE_INSTANCING", useInstancing ? "1" : "0" } };
    std::string vsPath = gShaderPathPrefix + "InstancingTestVS" + gShaderPathExt;
    mVertexShader.reset(CompileShader(vsPath.c_str(), ShaderType::Vertex, vsMacro, 1));
    if (!mVertexShader)
        return false;

    std::string psPath = gShaderPathPrefix + "InstancingTestPS" + gShaderPathExt;
    mPixelShader.reset(CompileShader(psPath.c_str(), ShaderType::Pixel, nullptr, 0));
    if (!mPixelShader)
        return false;

    ShaderProgramDesc shaderProgramDesc;
    shaderProgramDesc.vertexShader = mVertexShader.get();
    shaderProgramDesc.pixelShader = mPixelShader.get();
    mShaderProgram.reset(mRendererDevice->CreateShaderProgram(shaderProgramDesc));
    if (!mShaderProgram)
        return false;

    return true;
}

bool VertexBuffersScene::CreateBuffers(bool withInstanceBuffer)
{
    /// create vertex buffers
    BufferDesc vbDesc;
    vbDesc.type = BufferType::Vertex;
    vbDesc.access = BufferAccess::GPU_ReadOnly;


    float vbPositionData[] =
    {
        /// vertex structure: pos.xyz
        -0.05f, -0.05f, 0.0f,
         0.05f, -0.05f, 0.0f,
         0.05f,  0.05f, 0.0f,
        -0.05f,  0.05f, 0.0f,
    };

    vbDesc.size = sizeof(vbPositionData);
    vbDesc.initialData = vbPositionData;
    mPositionsVertexBuffer.reset(mRendererDevice->CreateBuffer(vbDesc));
    if (!mPositionsVertexBuffer)
        return false;


    float vbColorData[] =
    {
        /// vertex structure: color.rgba
        0.5f, 1.0f, 0.5f, 1.0f,
        0.0f, 0.5f, 1.0f, 1.0f,
        1.0f, 0.5f, 0.5f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
    };

    vbDesc.size = sizeof(vbColorData);
    vbDesc.initialData = vbColorData;
    mColorVertexBuffer.reset(mRendererDevice->CreateBuffer(vbDesc));
    if (!mColorVertexBuffer)
        return false;


    /// create index buffer
    uint16 ibData[] =
    {
        0, 1, 2,  // triangle 0
        0, 2, 3,  // triangle 1
    };

    BufferDesc ibDesc;
    ibDesc.type = BufferType::Index;
    ibDesc.access = BufferAccess::GPU_ReadOnly;
    ibDesc.size = sizeof(ibData);
    ibDesc.initialData = ibData;
    mIndexBuffer.reset(mRendererDevice->CreateBuffer(ibDesc));
    if (!mIndexBuffer)
        return false;


    /// create vertex layout object
    if (withInstanceBuffer)
    {
        Random random;
        std::unique_ptr<InstanceData[]> vbInstanceData(new InstanceData[gInstancesNumber]);

        for (int i = 0; i < gInstancesNumber; ++i)
        {
            vbInstanceData[i].pos.x = random.GetFloatBipolar();
            vbInstanceData[i].pos.y = random.GetFloatBipolar();
            vbInstanceData[i].pos.z = random.GetFloatBipolar();
            vbInstanceData[i].color = random.GetFloat4();
        }

        vbDesc.size = sizeof(InstanceData) * gInstancesNumber;
        vbDesc.initialData = vbInstanceData.get();
        mInstanceBuffer.reset(mRendererDevice->CreateBuffer(vbDesc));
        if (!mInstanceBuffer)
            return false;


        VertexLayoutElement vertexLayoutElements[] =
        {
            { ElementFormat::R32G32B32_Float,       0,  0, false, 0 }, // position
            { ElementFormat::R32G32B32A32_Float,    0,  1, false, 0 }, // color
            { ElementFormat::R32G32B32_Float,       0,  2, true,  1 }, // position offset
            { ElementFormat::R32G32B32A32_Float,    12, 2, true,  1 }, // color scale
        };

        VertexLayoutDesc vertexLayoutDesc;
        vertexLayoutDesc.elements = vertexLayoutElements;
        vertexLayoutDesc.numElements = 4;
        mVertexLayout.reset(mRendererDevice->CreateVertexLayout(vertexLayoutDesc));
    }
    else
    {
        VertexLayoutElement vertexLayoutElements[] =
        {
            { ElementFormat::R32G32B32_Float,       0, 0, false, 0 }, // position
            { ElementFormat::R32G32B32A32_Float,    0, 1, false, 0 }, // color
        };

        VertexLayoutDesc vertexLayoutDesc;
        vertexLayoutDesc.elements = vertexLayoutElements;
        vertexLayoutDesc.numElements = 2;
        mVertexLayout.reset(mRendererDevice->CreateVertexLayout(vertexLayoutDesc));
    }

    if (!mVertexLayout)
        return false;

    // create empty binding layout
    mResBindingLayout.reset(mRendererDevice->CreateResourceBindingLayout(
        ResourceBindingLayoutDesc()));
    if (!mResBindingLayout)
        return false;

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
    pipelineStateDesc.vertexLayout = mVertexLayout.get();
    pipelineStateDesc.raterizerState.cullMode = CullMode::Disabled;
    pipelineStateDesc.resBindingLayout = mResBindingLayout.get();
    mPipelineState.reset(mRendererDevice->CreatePipelineState(pipelineStateDesc));
    if (!mPipelineState)
        return false;

    return true;
}

/////////////////
/// Subscenes ///
/////////////////

bool VertexBuffersScene::CreateSubSceneSimple()
{
    if (!CreateShaderProgram(false))
        return false;

    if (!CreateBuffers(false))
        return false;

    return true;
}

bool VertexBuffersScene::CreateSubSceneInstancing()
{
    if (!CreateShaderProgram(true))
        return false;

    if (!CreateBuffers(true))
        return false;

    return true;
}

/////////////////////////////////////////////////////////
/// VertexBuffersScene methods and virtuals overriden ///
/////////////////////////////////////////////////////////

VertexBuffersScene::VertexBuffersScene()
    : Scene("VertexBuffers")
{
    RegisterSubScene(std::bind(&VertexBuffersScene::CreateSubSceneSimple, this),
                     "Simple (2 vertex buffers)");
    RegisterSubScene(std::bind(&VertexBuffersScene::CreateSubSceneInstancing, this),
                     "Instancing (3 vertex buffers)");
    // TODO: dynamic vertex buffer
}

VertexBuffersScene::~VertexBuffersScene()
{
    Release();
}

void VertexBuffersScene::ReleaseSubsceneResources()
{
    // clear resources

    mPositionsVertexBuffer.reset();
    mColorVertexBuffer.reset();
    mInstanceBuffer.reset();
    mIndexBuffer.reset();
    mVertexLayout.reset();
    mPipelineState.reset();

    mPixelShader.reset();
    mVertexShader.reset();
    mShaderProgram.reset();
    mResBindingLayout.reset();
}

bool VertexBuffersScene::OnInit(void* winHandle)
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

void VertexBuffersScene::Draw(float dt)
{
    // not used - the scene is static
    (void)dt;

    // reset bound resources and set them once again
    mCommandBuffer->Reset();
    mCommandBuffer->SetViewport(0.0f, static_cast<float>(WINDOW_WIDTH), 0.0f,
                                static_cast<float>(WINDOW_HEIGHT), 0.0f, 1.0f);
    mCommandBuffer->SetScissors(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    mCommandBuffer->SetRenderTarget(mWindowRenderTarget.get());

    // mCommandBuffer->SetResourceBindingLayout(mResBindingLayout.get());
    mCommandBuffer->SetPipelineState(mPipelineState.get());
    mCommandBuffer->SetShaderProgram(mShaderProgram.get());

    mCommandBuffer->SetIndexBuffer(mIndexBuffer.get(), IndexBufferFormat::Uint16);

    if (mInstanceBuffer)
    {
        IBuffer* vertexBuffers[] = { mPositionsVertexBuffer.get(),
                                     mColorVertexBuffer.get(),
                                     mInstanceBuffer.get() };
        int strides[] = { 3 * sizeof(float), 4 * sizeof(float), sizeof(InstanceData) };
        int offsets[] = { 0, 0, 0 };
        mCommandBuffer->SetVertexBuffers(3, vertexBuffers, strides, offsets);
    }
    else
    {
        IBuffer* vertexBuffers[] = { mPositionsVertexBuffer.get(), mColorVertexBuffer.get() };
        int strides[] = { 3 * sizeof(float), 4 * sizeof(float) };
        int offsets[] = { 0, 0 };
        mCommandBuffer->SetVertexBuffers(2, vertexBuffers, strides, offsets);
    }


    // clear target
    const Float4 color(0.0f, 0.0f, 0.0f, 1.0f);
    mCommandBuffer->Clear(ClearFlagsColor, 1, nullptr, &color);

    // draw
    if (mInstanceBuffer)
        mCommandBuffer->DrawIndexed(6, gInstancesNumber);
    else
        mCommandBuffer->DrawIndexed(6);

    mRendererDevice->Execute(mCommandBuffer->Finish().get());
    mWindowBackbuffer->Present();
}

void VertexBuffersScene::Release()
{
    ReleaseSubsceneResources();
    mWindowRenderTarget.reset();
    mWindowBackbuffer.reset();
    mCommandBuffer = nullptr;
    mRendererDevice = nullptr;
}