/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of high-level Postprocess Renderer
 */

#pragma once

#include "PCH.hpp"
#include "PostProcessRenderer.hpp"

namespace NFE {
namespace Renderer {

// renderer modules instance definition
std::unique_ptr<PostProcessRenderer> PostProcessRenderer::mPtr;

struct NFE_ALIGN16 ToneMappingCBuffer
{
    Vector bufferInvRes;
    Vector seed;
    Vector params;
};

PostProcessRenderer::PostProcessRenderer()
{
    IDevice* device = mRenderer->GetDevice();

    mTonemappingShaderProgram.Load("Tonemapping");

    CreateResourceBindingLayouts();

    /// create vertex layout
    VertexLayoutElement vertexLayoutElements[] =
    {
        { ElementFormat::Float_32, 3, 0, 0, false, 0 }, // position
    };
    VertexLayoutDesc vertexLayoutDesc;
    vertexLayoutDesc.elements = vertexLayoutElements;
    vertexLayoutDesc.numElements = 1;
    vertexLayoutDesc.debugName = "PostProcessRenderer::mVertexLayout";
    mVertexLayout.reset(device->CreateVertexLayout(vertexLayoutDesc));

    // vertices for full-screen quad
    Float3 vertices[] =
    {
        Float3(-1.0f, -1.0f, 0.0f),
        Float3(1.0f, -1.0f, 0.0f),
        Float3(1.0f, 1.0f, 0.0f),

        Float3(-1.0f, -1.0f, 0.0f),
        Float3(1.0f,  1.0f, 0.0f),
        Float3(-1.0f,  1.0f, 0.0f),
    };

    BufferDesc bufferDesc;
    bufferDesc.access = BufferAccess::GPU_ReadOnly;
    bufferDesc.size = sizeof(vertices);
    bufferDesc.type = BufferType::Vertex;
    bufferDesc.initialData = vertices;
    bufferDesc.debugName = "PostProcessRenderer::mVertexBuffer";
    mVertexBuffer.reset(device->CreateBuffer(bufferDesc));

    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.type = BufferType::Constant;
    bufferDesc.initialData = nullptr;
    bufferDesc.size = sizeof(ToneMappingCBuffer);
    bufferDesc.debugName = "PostProcessRenderer::mTonemappingCBuffer";
    mTonemappingCBuffer.reset(device->CreateBuffer(bufferDesc));

    mNullTextureBindingInstance = CreateTextureBinding(mRenderer->GetDefaultDiffuseTexture());

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.resBindingLayout = mResBindingLayout.get();
    pipelineStateDesc.raterizerState.cullMode = CullMode::Disabled;
    pipelineStateDesc.raterizerState.fillMode = FillMode::Solid;
    pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
    pipelineStateDesc.vertexLayout = mVertexLayout.get();
    pipelineStateDesc.debugName = "PostProcessRenderer::mPipelineState";
    mPipelineState.reset(device->CreatePipelineState(pipelineStateDesc));
}

bool PostProcessRenderer::CreateResourceBindingLayouts()
{
    IDevice* device = mRenderer->GetDevice();

    int paramsCBufferSlot = mTonemappingShaderProgram.GetResourceSlotByName("Params");
    if (paramsCBufferSlot < 0)
        return false;

    int sourceTextureSlot = mTonemappingShaderProgram.GetResourceSlotByName("gSourceTexture");
    if (sourceTextureSlot < 0)
        return false;

    DynamicBufferBindingDesc cbufferBindingDesc(ShaderType::Pixel, ShaderResourceType::CBuffer, paramsCBufferSlot);

    std::vector<IResourceBindingSet*> bindingSets;
    ResourceBindingDesc binding(ShaderResourceType::Texture, sourceTextureSlot,
                                 mRenderer->GetDefaultSampler());
    mTexturesBindingSet.reset(device->CreateResourceBindingSet(
        ResourceBindingSetDesc(&binding, 1, ShaderType::Pixel)));
    if (!mTexturesBindingSet)
        return false;
    bindingSets.push_back(mTexturesBindingSet.get());

    // create binding layout
    mResBindingLayout.reset(device->CreateResourceBindingLayout(
        ResourceBindingLayoutDesc(bindingSets.data(), bindingSets.size(), &cbufferBindingDesc, 1)));
    if (!mResBindingLayout)
        return false;

    return true;
}

std::unique_ptr<IResourceBindingInstance> PostProcessRenderer::CreateTextureBinding(ITexture* texture)
{
    IDevice* device = mRenderer->GetDevice();

    std::unique_ptr<IResourceBindingInstance> bindingInstance(
        device->CreateResourceBindingInstance(mTexturesBindingSet.get()));
    if (!bindingInstance)
        return std::unique_ptr<IResourceBindingInstance>();
    if (!bindingInstance->WriteTextureView(0, texture))
        return std::unique_ptr<IResourceBindingInstance>();

    return bindingInstance;
}

void PostProcessRenderer::OnEnter(RenderContext* context)
{
    context->commandBuffer->BeginDebugGroup("Post Process Renderer stage");

    context->commandBuffer->SetResourceBindingLayout(mResBindingLayout.get());
    context->commandBuffer->SetPipelineState(mPipelineState.get());

    IBuffer* veretexBuffers[] = { mVertexBuffer.get() };
    int strides[] = { sizeof(Float3) };
    int offsets[] = { 0 };
    context->commandBuffer->SetVertexBuffers(1, veretexBuffers, strides, offsets);
}

void PostProcessRenderer::OnLeave(RenderContext* context)
{
    context->commandBuffer->EndDebugGroup();
}

void PostProcessRenderer::ApplyTonemapping(RenderContext* context,
                                           const ToneMappingParameters& params,
                                           IResourceBindingInstance* src, IRenderTarget* dest)
{
    int width, height;
    dest->GetDimensions(width, height);
    context->commandBuffer->SetViewport(0.0f, static_cast<float>(width),
                                        0.0f, static_cast<float>(height),
                                        0.0f, 1.0f);
    context->commandBuffer->SetScissors(0, 0, width, height);

    context->commandBuffer->SetShaderProgram(mTonemappingShaderProgram.GetShaderProgram());

    ToneMappingCBuffer cbufferData;
    cbufferData.bufferInvRes = Vector(1.0f / static_cast<float>(width),
                                      1.0f / static_cast<float>(height));
    cbufferData.params = Vector(params.saturation,
                                params.noiseFactor,
                                expf(params.exposureOffset));
    cbufferData.seed = Vector(context->random.GetFloat2());

    context->commandBuffer->BindDynamicBuffer(0, mTonemappingCBuffer.get());
    context->commandBuffer->WriteBuffer(mTonemappingCBuffer.get(), 0,
                                        sizeof(cbufferData), &cbufferData);

    context->commandBuffer->SetRenderTarget(dest);
    context->commandBuffer->BindResources(0, src);


    // TODO: use compute shaders if supported
    context->commandBuffer->Draw(2 * 3);  // draw 2 triangles

    // unbind source texture
    context->commandBuffer->BindResources(0, mNullTextureBindingInstance.get());
}

} // namespace Renderer
} // namespace NFE
