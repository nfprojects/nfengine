/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  VertexBuffersScene definition
 */

#include "../PCH.hpp"

#include "VertexBuffers.hpp"
#include "../Common.hpp"

#include "Engine/Common/Math/Random.hpp"

#include <vector>
#include <functional>


using namespace NFE;
using namespace NFE::Math;
using namespace NFE::Renderer;

namespace {

int gInstancesNumber = 200;

static uint32 gFrameIndex = 0;

} // namespace



bool VertexBuffersScene::LoadShaders(bool useInstancing)
{
    ShaderMacro vsMacro[] = { { "USE_INSTANCING", useInstancing ? "1" : "0" } };
    const Common::String vsPath = gShaderPathPrefix + "InstancingTestVS" + gShaderPathExt;
    mVertexShader = CompileShader(vsPath.Str(), ShaderType::Vertex, vsMacro, 1);
    if (!mVertexShader)
        return false;

    const Common::String psPath = gShaderPathPrefix + "InstancingTestPS" + gShaderPathExt;
    mPixelShader = CompileShader(psPath.Str(), ShaderType::Pixel, nullptr, 0);
    if (!mPixelShader)
        return false;

    return true;
}

bool VertexBuffersScene::CreateBuffers(bool withInstanceBuffer, BufferMode vertexBufferMode)
{
    /// create vertex buffers
    BufferDesc vbDesc;
    vbDesc.type = BufferType::Vertex;
    vbDesc.mode = BufferMode::Static;


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
    mPositionsVertexBuffer = mRendererDevice->CreateBuffer(vbDesc);
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
    mColorVertexBuffer = mRendererDevice->CreateBuffer(vbDesc);
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
    ibDesc.mode = BufferMode::Static;
    ibDesc.size = sizeof(ibData);
    ibDesc.initialData = ibData;
    mIndexBuffer = mRendererDevice->CreateBuffer(ibDesc);
    if (!mIndexBuffer)
        return false;


    /// create vertex layout object
    if (withInstanceBuffer)
    {
        Random random;

        mInstancesData.resize(gInstancesNumber);
        mVelocities.resize(gInstancesNumber);

        for (int i = 0; i < gInstancesNumber; ++i)
        {
            mInstancesData[i].pos.x = random.GetFloatBipolar();
            mInstancesData[i].pos.y = random.GetFloatBipolar();
            mInstancesData[i].pos.z = random.GetFloatBipolar();
            mInstancesData[i].color = random.GetVec4fU();

            mVelocities[i].x = random.GetFloatBipolar();
            mVelocities[i].y = random.GetFloatBipolar();
        }

        mVertexBufferMode = vertexBufferMode;
        vbDesc.mode = vertexBufferMode;
        vbDesc.size = sizeof(InstanceData) * gInstancesNumber;
        vbDesc.initialData = mInstancesData.data();
        mInstanceBuffer = mRendererDevice->CreateBuffer(vbDesc);
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
        mVertexLayout = mRendererDevice->CreateVertexLayout(vertexLayoutDesc);
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
        mVertexLayout = mRendererDevice->CreateVertexLayout(vertexLayoutDesc);
    }

    if (!mVertexLayout)
        return false;

    // create empty binding layout
    mResBindingLayout = mRendererDevice->CreateResourceBindingLayout(ResourceBindingLayoutDesc());
    if (!mResBindingLayout)
        return false;

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.rtFormats[0] = ElementFormat::R8G8B8A8_U_Norm;
    pipelineStateDesc.vertexShader = mVertexShader;
    pipelineStateDesc.pixelShader = mPixelShader;
    pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
    pipelineStateDesc.vertexLayout = mVertexLayout;
    pipelineStateDesc.raterizerState.cullMode = CullMode::Disabled;
    pipelineStateDesc.resBindingLayout = mResBindingLayout;
    mPipelineState = mRendererDevice->CreatePipelineState(pipelineStateDesc);
    if (!mPipelineState)
        return false;

    return true;
}

/////////////////
/// Subscenes ///
/////////////////

bool VertexBuffersScene::CreateSubSceneSimple()
{
    gFrameIndex = 0;

    if (!LoadShaders(false))
        return false;

    if (!CreateBuffers(false, BufferMode::Static))
        return false;

    return true;
}

bool VertexBuffersScene::CreateSubSceneInstancing(BufferMode vertexBufferMode)
{
    gFrameIndex = 0;

    if (!LoadShaders(true))
        return false;

    if (!CreateBuffers(true, vertexBufferMode))
        return false;

    return true;
}

/////////////////////////////////////////////////////////
/// VertexBuffersScene methods and virtuals overridden ///
/////////////////////////////////////////////////////////

VertexBuffersScene::VertexBuffersScene()
    : Scene("VertexBuffers")
{
    RegisterSubScene(std::bind(&VertexBuffersScene::CreateSubSceneSimple, this),
                     "Simple: Static vertex buffer (2 vertex buffers)");
    RegisterSubScene(std::bind(&VertexBuffersScene::CreateSubSceneInstancing, this, BufferMode::Static),
                     "Instancing: Static vertex buffer (3 vertex buffers)");
    RegisterSubScene(std::bind(&VertexBuffersScene::CreateSubSceneInstancing, this, BufferMode::Dynamic),
                     "Instancing: Dynamic vertex buffer (3 vertex buffers)");
    RegisterSubScene(std::bind(&VertexBuffersScene::CreateSubSceneInstancing, this, BufferMode::Volatile),
                     "Instancing: Volatile vertex buffer (3 vertex buffers)");
}

VertexBuffersScene::~VertexBuffersScene()
{
    Release();
}

void VertexBuffersScene::ReleaseSubsceneResources()
{
    Scene::ReleaseSubsceneResources();

    // clear resources

    mPositionsVertexBuffer.Reset();
    mColorVertexBuffer.Reset();
    mInstanceBuffer.Reset();
    mIndexBuffer.Reset();
    mVertexLayout.Reset();
    mPipelineState.Reset();

    mPixelShader.Reset();
    mVertexShader.Reset();
    mResBindingLayout.Reset();
}

bool VertexBuffersScene::OnInit(void* winHandle)
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

void VertexBuffersScene::Draw(float dt)
{
    // reset bound resources and set them once again
    mCommandBuffer->Begin();
    mCommandBuffer->SetViewport(0.0f, static_cast<float>(WINDOW_WIDTH), 0.0f,
                                static_cast<float>(WINDOW_HEIGHT), 0.0f, 1.0f);
    mCommandBuffer->SetScissors(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    mCommandBuffer->SetRenderTarget(mWindowRenderTarget);

    mCommandBuffer->SetPipelineState(mPipelineState);

    mCommandBuffer->SetIndexBuffer(mIndexBuffer, IndexBufferFormat::Uint16);

    if (mInstanceBuffer)
    {
        const BufferPtr vertexBuffers[] = { mPositionsVertexBuffer, mColorVertexBuffer, mInstanceBuffer };
        uint32 strides[] = { 3 * sizeof(float), 4 * sizeof(float), sizeof(InstanceData) };
        uint32 offsets[] = { 0, 0, 0 };
        mCommandBuffer->SetVertexBuffers(3, vertexBuffers, strides, offsets);

        // update dynamic/volatile buffer
        if (mVertexBufferMode == BufferMode::Dynamic || mVertexBufferMode == BufferMode::Volatile)
        {
            for (int i = 0; i < gInstancesNumber; ++i)
            {
                const float velocity = 0.3f;
                mInstancesData[i].pos.x += (velocity * dt) * mVelocities[i].x;
                mInstancesData[i].pos.y += (velocity * dt) * mVelocities[i].y;

                if (mInstancesData[i].pos.x > 1.0f || mInstancesData[i].pos.x < -1.0f)
                    mVelocities[i].x = -mVelocities[i].x;

                if (mInstancesData[i].pos.y > 1.0f || mInstancesData[i].pos.y < -1.0f)
                    mVelocities[i].y = -mVelocities[i].y;
            }

            mCommandBuffer->WriteBuffer(mInstanceBuffer, 0, sizeof(InstanceData) * gInstancesNumber, mInstancesData.data());
        }
    }
    else
    {
        const BufferPtr vertexBuffers[] = { mPositionsVertexBuffer, mColorVertexBuffer };
        uint32 strides[] = { 3 * sizeof(float), 4 * sizeof(float) };
        uint32 offsets[] = { 0, 0 };
        mCommandBuffer->SetVertexBuffers(2, vertexBuffers, strides, offsets);
    }

    if (gFrameIndex == 0)
    {
        // clear target
        const Vec4fU color(0.0f, 0.0f, 0.0f, 1.0f);
        mCommandBuffer->Clear(ClearFlagsColor, 1, nullptr, &color);
    }

    // draw
    if (mInstanceBuffer)
        mCommandBuffer->DrawIndexed(6, gInstancesNumber);
    else
        mCommandBuffer->DrawIndexed(6);

    mCommandBuffer->CopyTexture(mWindowRenderTargetTexture, mWindowBackbuffer);

    CommandListID commandList = mCommandBuffer->Finish();
    mRendererDevice->Execute(commandList);
    mWindowBackbuffer->Present();
    mRendererDevice->FinishFrame();

    gFrameIndex++;
}

void VertexBuffersScene::Release()
{
    ReleaseSubsceneResources();
    mWindowRenderTarget.Reset();
    mWindowBackbuffer.Reset();
    mCommandBuffer.Reset();
    mRendererDevice = nullptr;
}
