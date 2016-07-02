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
#include "../../nfCommon/Logger.hpp"

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

const size_t gMaxBufferedInstances = 4096;

} // namespace

// renderer modules instance definition
std::unique_ptr<GeometryRenderer> GeometryRenderer::mPtr;


GBufferRendererContext::GBufferRendererContext()
{
    instanceData.resize(gMaxBufferedInstances);
}

GeometryRenderer::GeometryRenderer()
{
    IDevice* device = mRenderer->GetDevice();

    BufferDesc bufferDesc;

    mGeometryPassShaderProgram.Load("GeometryPass");
    mShadowShaderProgram.Load("Shadow");

    mUseMotionBlurMacroId = mGeometryPassShaderProgram.GetMacroByName("USE_MOTION_BLUR");
    mCubeShadowMapMacroId = mShadowShaderProgram.GetMacroByName("CUBE_SHADOW_MAP");

    /// create vertex layout
    VertexLayoutElement vertexLayoutElements[] =
    {
        /// per-vertex data:
        { ElementFormat::Float_32,   3,  0, 0, false, 0 }, // position
        { ElementFormat::Float_32,   2, 12, 0, false, 0 }, // tex-coords
        { ElementFormat::Int_8_norm, 4, 20, 0, false, 0 }, // normal
        { ElementFormat::Int_8_norm, 4, 24, 0, false, 0 }, // tangent
        // per-instance data:
        { ElementFormat::Float_32, 4,  0, 1, true, 1 },
        { ElementFormat::Float_32, 4, 16, 1, true, 1 },
        { ElementFormat::Float_32, 4, 32, 1, true, 1 },
        { ElementFormat::Float_32, 4, 48, 1, true, 1 },
        { ElementFormat::Float_32, 4, 64, 1, true, 1 },
    };

    VertexLayoutDesc meshVertexLayoutDesc;
    meshVertexLayoutDesc.elements = vertexLayoutElements;
    meshVertexLayoutDesc.numElements = 9;
    meshVertexLayoutDesc.debugName = "GeometryRenderer::mMeshVertexLayout";
    mVertexLayout.reset(device->CreateVertexLayout(meshVertexLayoutDesc));

    // create vertex buffer for per-instance data
    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = gMaxBufferedInstances * sizeof(InstanceData);
    bufferDesc.type = BufferType::Vertex;
    bufferDesc.debugName = "GeometryRenderer::mInstancesVertexBuffer";
    mInstancesVertexBuffer.reset(device->CreateBuffer(bufferDesc));

    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = sizeof(MaterialCBuffer);
    bufferDesc.type = BufferType::Constant;
    bufferDesc.debugName = "GeometryRenderer::mMaterialCBuffer";
    mMaterialCBuffer.reset(device->CreateBuffer(bufferDesc));

    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = sizeof(GlobalCBuffer);
    bufferDesc.type = BufferType::Constant;
    bufferDesc.debugName = "GeometryRenderer::mGlobalCBuffer";
    mGlobalCBuffer.reset(device->CreateBuffer(bufferDesc));

    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = sizeof(ShadowCameraRenderDesc);
    bufferDesc.type = BufferType::Constant;
    bufferDesc.debugName = "GeometryRenderer::mShadowGlobalCBuffer";
    mShadowGlobalCBuffer.reset(device->CreateBuffer(bufferDesc));

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.depthState.depthCompareFunc = CompareFunc::Less;
    pipelineStateDesc.depthState.depthTestEnable = true;
    pipelineStateDesc.depthState.depthWriteEnable = true;
    pipelineStateDesc.raterizerState.cullMode = CullMode::CW;
    pipelineStateDesc.raterizerState.fillMode = FillMode::Solid;
    pipelineStateDesc.vertexLayout = mVertexLayout.get();
    pipelineStateDesc.debugName = "GeometryRenderer::mPipelineState";
    mPipelineState.reset(device->CreatePipelineState(pipelineStateDesc));
}

void GeometryRenderer::OnEnter(RenderContext* context)
{
    context->commandBuffer->BeginDebugGroup("Geometry Buffer Renderer stage");

    context->commandBuffer->SetPipelineState(mPipelineState.get());

    ISampler* sampler = mRenderer->GetDefaultSampler();
    context->commandBuffer->SetSamplers(&sampler, 1, ShaderType::Pixel);
}

void GeometryRenderer::OnLeave(RenderContext* context)
{
    // TODO: allow "NULL" in the array
    ITexture* nullTextures[] = { mRenderer->GetDefaultDiffuseTexture(),
        mRenderer->GetDefaultDiffuseTexture(),
        mRenderer->GetDefaultDiffuseTexture(),
        mRenderer->GetDefaultDiffuseTexture(),
        mRenderer->GetDefaultDiffuseTexture(),
        mRenderer->GetDefaultDiffuseTexture() };
    context->commandBuffer->SetTextures(nullTextures, 6, ShaderType::Pixel);

    context->commandBuffer->EndDebugGroup();
}

void GeometryRenderer::SetUp(RenderContext* context, GeometryBuffer* geometryBuffer,
                             const CameraRenderDesc* cameraDesc)
{
    context->commandBuffer->InsertDebugMarker(__FUNCTION__);

    context->commandBuffer->SetRenderTarget(geometryBuffer->mRenderTarget.get());
    context->commandBuffer->SetViewport(0.0f, static_cast<float>(geometryBuffer->mWidth),
                                        0.0f, static_cast<float>(geometryBuffer->mHeight),
                                        0.0f, 1.0f);

    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    context->commandBuffer->Clear(NFE_CLEAR_FLAG_TARGET | NFE_CLEAR_FLAG_DEPTH, clearColor, 1.0f);

    int macros[] = { 0 }; // USE_MOTION_BLUR
    context->commandBuffer->SetShaderProgram(mGeometryPassShaderProgram.GetShaderProgram(macros));

    IBuffer* vsConstantBuffers[] = { mGlobalCBuffer.get() };
    context->commandBuffer->SetConstantBuffers(vsConstantBuffers, 1, ShaderType::Vertex);
    IBuffer* psConstantBuffers[] = { mGlobalCBuffer.get(), mMaterialCBuffer.get() };
    context->commandBuffer->SetConstantBuffers(psConstantBuffers, 2, ShaderType::Pixel);

    GlobalCBuffer cbuffer;
    cbuffer.ProjMatrix = cameraDesc->projMatrix;
    cbuffer.ViewMatrix = cameraDesc->viewMatrix;
    cbuffer.SecondaryViewProjMatrix = cameraDesc->secViewMatrix;
    cbuffer.ViewProjMatrix = cameraDesc->viewMatrix * cameraDesc->projMatrix;
    cbuffer.CameraVelocity = cameraDesc->velocity;
    cbuffer.CameraAngularVelocity = cameraDesc->angualrVelocity;

    context->commandBuffer->WriteBuffer(mGlobalCBuffer.get(), 0, sizeof(GlobalCBuffer),
                                        &cbuffer);
}

void GeometryRenderer::SetUpForShadowMap(RenderContext *context, ShadowMap* shadowMap,
                                         const ShadowCameraRenderDesc* cameraDesc,
                                         uint32 faceID)
{
    context->commandBuffer->InsertDebugMarker(__FUNCTION__);

    context->commandBuffer->SetRenderTarget(shadowMap->mRenderTargets[faceID].get());
    context->commandBuffer->SetViewport(0.0f, static_cast<float>(shadowMap->mSize),
                                        0.0f, static_cast<float>(shadowMap->mSize),
                                        0.0f, 1.0f);

    const float clearColor[] = { 1.0f, 0.0f, 0.0f, 0.0f };
    context->commandBuffer->Clear(NFE_CLEAR_FLAG_TARGET | NFE_CLEAR_FLAG_DEPTH, clearColor, 1.0f);

    int macros[] = { 0 };
    if (shadowMap->mType == ShadowMap::Type::Cube)
        macros[0] = 1;

    context->commandBuffer->SetShaderProgram(mShadowShaderProgram.GetShaderProgram(macros));

    IBuffer* constantBuffers[] = { mShadowGlobalCBuffer.get() };
    context->commandBuffer->SetConstantBuffers(constantBuffers, 1, ShaderType::Vertex);
    context->commandBuffer->SetConstantBuffers(constantBuffers, 1, ShaderType::Pixel);

    context->commandBuffer->WriteBuffer(mShadowGlobalCBuffer.get(), 0,
                                        sizeof(ShadowCameraRenderDesc), cameraDesc);
}

void GeometryRenderer::SetMaterial(RenderContext* context, const RendererMaterial* material)
{
    // TODO: use additional macros/branching in the shaders instead of using "dummy" textures
    ITexture* diffuseTexture = mRenderer->GetDefaultDiffuseTexture();
    ITexture* normalTexture = mRenderer->GetDefaultNormalTexture();
    ITexture* specularTexture = mRenderer->GetDefaultSpecularTexture();

    MaterialCBuffer cbuffer;
    cbuffer.diffuseColor = Vector(1.0f, 1.0f, 1.0f, 1.0f);
    cbuffer.specularColor = Vector(1.0f, 1.0f, 1.0f, 1.0f);
    cbuffer.emissionColor = Vector();

    // TODO: multi-layer textures
    if (material)
    {
        RendererMaterialLayer* layer = &material->layers[0];
        if (layer)
        {
            if (layer->diffuseTex != nullptr)
                diffuseTexture = layer->diffuseTex;
            if (layer->normalTex != nullptr)
                normalTexture = layer->normalTex;
            if (layer->specularTex != nullptr)
                specularTexture = layer->specularTex;

            cbuffer.diffuseColor = layer->diffuseColor;
            cbuffer.specularColor = layer->specularColor;
            cbuffer.emissionColor = layer->emissionColor;
        }
    }

    context->commandBuffer->WriteBuffer(mMaterialCBuffer.get(), 0, sizeof(MaterialCBuffer),
                                        &cbuffer);

    ITexture* textures[] = { diffuseTexture, normalTexture, specularTexture };
    context->commandBuffer->SetTextures(textures, 3, ShaderType::Pixel);
}

void GeometryRenderer::Draw(RenderContext* context, const RenderCommandBuffer& buffer)
{
    if (buffer.commands.size() <= 0)
        return;

    GBufferRendererContext& ctx = context->geometryBufferContext;

    IBuffer* currVB = nullptr;
    IBuffer* currIB = nullptr;
    uint32 currStartIndex = 0xFFFFFFFF;
    uint32 currIndexCount = 0;

    const RendererMaterial* currMaterial = nullptr;
    int bufferedInstances = 0;
    uint32 startInstanceLocation = 0;

    for (size_t i = 0; i < buffer.commands.size(); i++)
    {
        const RenderCommand& command = buffer.commands[i];

        bool bufferIsFull = (bufferedInstances + bufferedInstances >= gMaxBufferedInstances) || (i == 0);
        bool materialChange = (command.material != currMaterial);
        bool meshChange = ((currIB != command.pIB) || (currVB != command.pVB) ||
                           (currStartIndex != command.startIndex));

        // flush buffered instances
        if (materialChange || meshChange || bufferIsFull)
        {
            if (bufferedInstances > 0)
            {
                context->commandBuffer->DrawIndexed(PrimitiveType::Triangles, currIndexCount,
                                                    bufferedInstances, currStartIndex, 0,
                                                    startInstanceLocation);
                startInstanceLocation += bufferedInstances;
                bufferedInstances = 0;
            }
        }

        // instances buffer is full
        if (bufferIsFull)
        {
            // udapte per instance data buffer
            size_t instancesToBuffer = Min<size_t>(gMaxBufferedInstances, buffer.commands.size() - i);

            for (size_t j = 0; j < instancesToBuffer; j++)
            {
                Matrix tmpMatrix = MatrixTranspose(buffer.commands[j + i].matrix);
                ctx.instanceData[j].worldMatrix[0] = tmpMatrix.r[0];
                ctx.instanceData[j].worldMatrix[1] = tmpMatrix.r[1];
                ctx.instanceData[j].worldMatrix[2] = tmpMatrix.r[2];
                ctx.instanceData[j].velocity = buffer.commands[j + i].velocity;
                ctx.instanceData[j].angularVelocity = buffer.commands[j + i].angularVelocity;
            }

            context->commandBuffer->WriteBuffer(mInstancesVertexBuffer.get(),
                                                0, instancesToBuffer * sizeof(InstanceData),
                                                ctx.instanceData.data());

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

            IBuffer* buffers[] = { currVB, mInstancesVertexBuffer.get() };
            int strides[] = { sizeof(MeshVertex), sizeof(InstanceData) };
            int offsets[] = { 0, 0 };
            context->commandBuffer->SetVertexBuffers(2, buffers, strides, offsets);
            context->commandBuffer->SetIndexBuffer(currIB, IndexBufferFormat::Uint32);
        }

        bufferedInstances++;
    }

    // flush the rest
    if (bufferedInstances > 0)
    {
        context->commandBuffer->DrawIndexed(PrimitiveType::Triangles, currIndexCount,
                                            bufferedInstances, currStartIndex, 0,
                                            startInstanceLocation);
    }
}

} // namespace Renderer
} // namespace NFE
