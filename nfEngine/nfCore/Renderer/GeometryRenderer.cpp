/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of high-level Geometry Renderer
 */

#pragma once

#include "PCH.hpp"
#include "GeometryRenderer.hpp"
#include "HighLevelRenderer.hpp"
#include "Resources/Material.hpp"
#include "Resources/Mesh.hpp"
#include "ShaderCommon.hpp"
#include "nfCommon/Logger/Logger.hpp"

namespace NFE {
namespace Renderer {

namespace {

struct GlobalCBuffer
{
    float4x4 ViewMatrix;
    float4x4 ProjMatrix;
    float4x4 ViewProjMatrix;
    float4x4 SecondaryViewProjMatrix;
    float4 CameraVelocity;
    float4 CameraAngularVelocity;
};


struct MaterialCBuffer
{
    Vector diffuseColor;
    Vector specularColor;
    Vector emissionColor;
};

} // namespace

// renderer modules instance definition
std::unique_ptr<GeometryRenderer> GeometryRenderer::mPtr;

const size_t GeometryRendererContext::gMaxBufferedInstances = 4096;


GeometryRenderer::GeometryRenderer()
{
    IDevice* device = mRenderer->GetDevice();

    BufferDesc bufferDesc;

    mGeometryPassPipelineState.Load("GeometryPass");
    mShadowPipelineState.Load("Shadow");

    mUseMotionBlurMacroId = mGeometryPassPipelineState.GetMacroByName("USE_MOTION_BLUR");
    mCubeShadowMapMacroId = mShadowPipelineState.GetMacroByName("CUBE_SHADOW_MAP");

    CreateResourceBindingLayouts();

    /// create vertex layout
    VertexLayoutElement vertexLayoutElements[] =
    {
        /// per-vertex data:
        { ElementFormat::R32G32B32_Float,   0,  0, false, 0 }, // position
        { ElementFormat::R32G32_Float,      12, 0, false, 0 }, // tex-coords
        { ElementFormat::R8G8B8A8_S_Norm,   20, 0, false, 0 }, // normal
        { ElementFormat::R8G8B8A8_S_Norm,   24, 0, false, 0 }, // tangent
        // per-instance data:
        { ElementFormat::R32G32B32A32_Float,  0, 1, true, 1 },
        { ElementFormat::R32G32B32A32_Float, 16, 1, true, 1 },
        { ElementFormat::R32G32B32A32_Float, 32, 1, true, 1 },
        { ElementFormat::R32G32B32A32_Float, 48, 1, true, 1 },
        { ElementFormat::R32G32B32A32_Float, 64, 1, true, 1 },
    };

    VertexLayoutDesc meshVertexLayoutDesc;
    meshVertexLayoutDesc.elements = vertexLayoutElements;
    meshVertexLayoutDesc.numElements = 9;
    meshVertexLayoutDesc.debugName = "GeometryRenderer::mMeshVertexLayout";
    mVertexLayout = device->CreateVertexLayout(meshVertexLayoutDesc);

    // create vertex buffer for per-instance data
    bufferDesc.mode = BufferMode::Dynamic;
    bufferDesc.size = GeometryRendererContext::gMaxBufferedInstances * sizeof(InstanceData);
    bufferDesc.type = BufferType::Vertex;
    bufferDesc.debugName = "GeometryRenderer::mInstancesVertexBuffer";
    mInstancesVertexBuffer = device->CreateBuffer(bufferDesc);

    bufferDesc.mode = BufferMode::Volatile;
    bufferDesc.size = sizeof(MaterialCBuffer);
    bufferDesc.type = BufferType::Constant;
    bufferDesc.debugName = "GeometryRenderer::mMaterialCBuffer";
    mMaterialCBuffer = device->CreateBuffer(bufferDesc);

    bufferDesc.mode = BufferMode::Volatile;
    bufferDesc.size = sizeof(GlobalCBuffer);
    bufferDesc.type = BufferType::Constant;
    bufferDesc.debugName = "GeometryRenderer::mGlobalCBuffer";
    mGlobalCBuffer = device->CreateBuffer(bufferDesc);

    bufferDesc.mode = BufferMode::Volatile;
    bufferDesc.size = sizeof(ShadowCameraRenderDesc);
    bufferDesc.type = BufferType::Constant;
    bufferDesc.debugName = "GeometryRenderer::mShadowGlobalCBuffer";
    mShadowGlobalCBuffer = device->CreateBuffer(bufferDesc);

    // dummy material (with default textures) binding instance
    mDummyMaterialBindingInstance = device->CreateResourceBindingInstance(mMatTexturesBindingSet);
    if (mDummyMaterialBindingInstance)
    {
        mDummyMaterialBindingInstance->WriteTextureView(0, mRenderer->GetDefaultDiffuseTexture());
        mDummyMaterialBindingInstance->WriteTextureView(1, mRenderer->GetDefaultNormalTexture());
        mDummyMaterialBindingInstance->WriteTextureView(2, mRenderer->GetDefaultSpecularTexture());
    }

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.resBindingLayout = mResBindingLayout;
    pipelineStateDesc.depthState.depthCompareFunc = CompareFunc::Less;
    pipelineStateDesc.depthState.depthTestEnable = true;
    pipelineStateDesc.depthState.depthWriteEnable = true;
    pipelineStateDesc.raterizerState.cullMode = CullMode::CW;
    pipelineStateDesc.raterizerState.fillMode = FillMode::Solid;
    pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
    pipelineStateDesc.vertexLayout = mVertexLayout;

    pipelineStateDesc.debugName = "GeometryRenderer::mShadowPipelineState";
    pipelineStateDesc.depthFormat = DepthBufferFormat::Depth32;
    pipelineStateDesc.rtFormats[0] = ElementFormat::R32_Float;
    pipelineStateDesc.numRenderTargets = 1;
    mShadowPipelineState.Build(pipelineStateDesc);

    pipelineStateDesc.debugName = "GeometryRenderer::mGeometryPassPipelineState";
    pipelineStateDesc.depthFormat = DepthBufferFormat::Depth32;
    pipelineStateDesc.rtFormats[0] = ElementFormat::R16G16B16A16_Float;
    pipelineStateDesc.rtFormats[1] = ElementFormat::R16G16B16A16_Float;
    pipelineStateDesc.rtFormats[2] = ElementFormat::R16G16B16A16_Float;
    pipelineStateDesc.rtFormats[3] = ElementFormat::R16G16_Float;
    pipelineStateDesc.numRenderTargets = 4;
    mGeometryPassPipelineState.Build(pipelineStateDesc);
}

bool GeometryRenderer::CreateResourceBindingLayouts()
{
    IDevice* device = mRenderer->GetDevice();

    int globalCBufferSlot = mGeometryPassPipelineState.GetResourceSlotByName("Global");
    if (globalCBufferSlot < 0)
        return false;

    int materialCBufferSlot = mGeometryPassPipelineState.GetResourceSlotByName("Material");
    if (materialCBufferSlot < 0)
        return false;

    int diffuseTextureSlot  = mGeometryPassPipelineState.GetResourceSlotByName("gDiffuseTexture");
    int normalTextureSlot   = mGeometryPassPipelineState.GetResourceSlotByName("gNormalTexture");
    int specularTextureSlot = mGeometryPassPipelineState.GetResourceSlotByName("gSpecularTexture");
    if (materialCBufferSlot < 0 || normalTextureSlot < 0 || specularTextureSlot < 0)
        return false;

    std::vector<ResourceBindingSetPtr> bindingSets;

    ResourceBindingDesc binding2[3] =
    {
        ResourceBindingDesc(ShaderResourceType::Texture,
                            diffuseTextureSlot,
                            mRenderer->GetDefaultSampler()),
        ResourceBindingDesc(ShaderResourceType::Texture,
                            normalTextureSlot,
                            mRenderer->GetDefaultSampler()),
        ResourceBindingDesc(ShaderResourceType::Texture,
                            specularTextureSlot,
                            mRenderer->GetDefaultSampler()),
    };
    mMatTexturesBindingSet = device->CreateResourceBindingSet(ResourceBindingSetDesc(binding2, 3, ShaderType::Pixel));
    if (!mMatTexturesBindingSet)
        return false;
    bindingSets.push_back(mMatTexturesBindingSet);

    VolatileCBufferBinding cbufferBindingsDesc[2] =
    {
        VolatileCBufferBinding(ShaderType::All, ShaderResourceType::CBuffer, globalCBufferSlot, sizeof(GlobalCBuffer)),
        VolatileCBufferBinding(ShaderType::Pixel, ShaderResourceType::CBuffer, materialCBufferSlot, sizeof(MaterialCBuffer)),
    };

    // create binding layout
    mResBindingLayout = device->CreateResourceBindingLayout(
        ResourceBindingLayoutDesc(bindingSets.data(), bindingSets.size(), cbufferBindingsDesc, 2));
    if (!mResBindingLayout)
        return false;

    return true;
}

void GeometryRenderer::OnEnter(GeometryRendererContext* context)
{
    context->commandRecorder->BeginDebugGroup("Geometry Buffer Renderer stage");
}

void GeometryRenderer::OnLeave(GeometryRendererContext* context)
{
    context->commandRecorder->SetRenderTarget(nullptr);
    context->commandRecorder->EndDebugGroup();
}

void GeometryRenderer::SetUp(GeometryRendererContext* context, GeometryBuffer* geometryBuffer,
                             const CameraRenderDesc* cameraDesc)
{
    context->commandRecorder->InsertDebugMarker(__FUNCTION__);

    context->commandRecorder->SetResourceBindingLayout(mResBindingLayout);
    context->commandRecorder->SetRenderTarget(geometryBuffer->mRenderTarget);
    context->commandRecorder->SetViewport(0.0f, static_cast<float>(geometryBuffer->mWidth),
                                        0.0f, static_cast<float>(geometryBuffer->mHeight),
                                        0.0f, 1.0f);
    context->commandRecorder->SetScissors(0, 0, geometryBuffer->mWidth, geometryBuffer->mHeight);

    context->commandRecorder->Clear(ClearFlagsDepth, 0, nullptr, nullptr, 1.0f);

    int macros[] = { 0 }; // USE_MOTION_BLUR
    context->commandRecorder->SetPipelineState(mGeometryPassPipelineState.GetPipelineState(macros));

    context->commandRecorder->BindVolatileCBuffer(0, mGlobalCBuffer);
    context->commandRecorder->BindVolatileCBuffer(1, mMaterialCBuffer);

    GlobalCBuffer cbuffer;
    cbuffer.ProjMatrix = cameraDesc->projMatrix;
    cbuffer.ViewMatrix = cameraDesc->viewMatrix;
    cbuffer.SecondaryViewProjMatrix = cameraDesc->secViewMatrix;
    cbuffer.ViewProjMatrix = cameraDesc->viewMatrix * cameraDesc->projMatrix;
    cbuffer.CameraVelocity = cameraDesc->velocity;
    cbuffer.CameraAngularVelocity = cameraDesc->angualrVelocity;

    context->commandRecorder->WriteBuffer(mGlobalCBuffer, 0, sizeof(GlobalCBuffer),
                                        &cbuffer);
}

void GeometryRenderer::SetUpForShadowMap(GeometryRendererContext *context, ShadowMap* shadowMap,
                                         const ShadowCameraRenderDesc* cameraDesc,
                                         uint32 faceID)
{
    context->commandRecorder->InsertDebugMarker(__FUNCTION__);

    context->commandRecorder->SetResourceBindingLayout(mResBindingLayout);
    context->commandRecorder->SetRenderTarget(shadowMap->mRenderTargets[faceID]);
    context->commandRecorder->SetViewport(0.0f, static_cast<float>(shadowMap->mSize),
                                        0.0f, static_cast<float>(shadowMap->mSize),
                                        0.0f, 1.0f);
    context->commandRecorder->SetScissors(0, 0, shadowMap->mSize, shadowMap->mSize);

    const Float4 clearColor(1.0f, 0.0f, 0.0f, 0.0f);
    context->commandRecorder->Clear(ClearFlagsColor | ClearFlagsDepth, 1, nullptr, &clearColor, 1.0f);

    int macros[] = { 0 };
    if (shadowMap->mType == ShadowMap::Type::Cube)
        macros[0] = 1;

    context->commandRecorder->SetPipelineState(mShadowPipelineState.GetPipelineState(macros));

    context->commandRecorder->BindVolatileCBuffer(0, mShadowGlobalCBuffer);

    context->commandRecorder->WriteBuffer(mShadowGlobalCBuffer, 0,
                                        sizeof(ShadowCameraRenderDesc), cameraDesc);
}

void GeometryRenderer::SetMaterial(GeometryRendererContext* context, const RendererMaterial* material)
{
    ResourceBindingInstancePtr bindingInstance = mDummyMaterialBindingInstance;
    if (material && material->layers[0].bindingInstance)
        bindingInstance = material->layers[0].bindingInstance;

    context->commandRecorder->BindResources(0, bindingInstance);

    MaterialCBuffer cbuffer;
    cbuffer.diffuseColor = Vector(1.0f, 1.0f, 1.0f, 1.0f);
    cbuffer.specularColor = Vector(1.0f, 1.0f, 1.0f, 1.0f);
    cbuffer.emissionColor = Vector();
    context->commandRecorder->WriteBuffer(mMaterialCBuffer, 0, sizeof(MaterialCBuffer),
                                        &cbuffer);
}

void GeometryRenderer::Draw(GeometryRendererContext* context, const RenderCommandBuffer& buffer)
{
    if (buffer.commands.size() <= 0)
        return;

    BufferPtr currVB = nullptr;
    BufferPtr currIB = nullptr;
    uint32 currStartIndex = 0xFFFFFFFF;
    uint32 currIndexCount = 0;

    const RendererMaterial* currMaterial = nullptr;
    int bufferedInstances = 0;
    uint32 startInstanceLocation = 0;

    for (size_t i = 0; i < buffer.commands.size(); i++)
    {
        const RenderCommand& command = buffer.commands[i];

        bool bufferIsFull = (bufferedInstances + bufferedInstances >= GeometryRendererContext::gMaxBufferedInstances) || (i == 0);
        bool materialChange = (command.material != currMaterial);
        bool meshChange = ((currIB != command.pIB) || (currVB != command.pVB) ||
                           (currStartIndex != command.startIndex));

        // flush buffered instances
        if (materialChange || meshChange || bufferIsFull)
        {
            if (bufferedInstances > 0)
            {
                context->commandRecorder->DrawIndexed(currIndexCount,
                                                    bufferedInstances, currStartIndex, 0,
                                                    startInstanceLocation);
                startInstanceLocation += bufferedInstances;
                bufferedInstances = 0;
            }
        }

        // instances buffer is full
        if (bufferIsFull)
        {
            // update per instance data buffer
            size_t instancesToBuffer = Min<size_t>(GeometryRendererContext::gMaxBufferedInstances, buffer.commands.size() - i);

            for (size_t j = 0; j < instancesToBuffer; j++)
            {
                const Matrix tmpMatrix = buffer.commands[j + i].matrix.Transposed();
                context->instanceData[j].worldMatrix[0] = tmpMatrix.r[0];
                context->instanceData[j].worldMatrix[1] = tmpMatrix.r[1];
                context->instanceData[j].worldMatrix[2] = tmpMatrix.r[2];
                context->instanceData[j].velocity = buffer.commands[j + i].velocity;
                context->instanceData[j].angularVelocity = buffer.commands[j + i].angularVelocity;
            }

            context->commandRecorder->WriteBuffer(mInstancesVertexBuffer,
                                                0, instancesToBuffer * sizeof(InstanceData),
                                                context->instanceData.data());

            bufferedInstances = startInstanceLocation = 0;
        }

        // material has changed
        if (materialChange)
        {
            currMaterial = command.material;
            SetMaterial(context, currMaterial);
        }

        // mesh has changed
        if (meshChange)
        {
            currStartIndex = command.startIndex;
            currIndexCount = command.indexCount;
            currIB = command.pIB;
            currVB = command.pVB;

            const BufferPtr buffers[] = { currVB, mInstancesVertexBuffer };
            int strides[] = { sizeof(Resource::MeshVertex), sizeof(InstanceData) };
            int offsets[] = { 0, 0 };
            context->commandRecorder->SetVertexBuffers(2, buffers, strides, offsets);
            context->commandRecorder->SetIndexBuffer(currIB, IndexBufferFormat::Uint32);
        }

        bufferedInstances++;
    }

    // flush the rest
    if (bufferedInstances > 0)
    {
        context->commandRecorder->DrawIndexed(currIndexCount,
                                            bufferedInstances, currStartIndex, 0,
                                            startInstanceLocation);
    }
}

} // namespace Renderer
} // namespace NFE
