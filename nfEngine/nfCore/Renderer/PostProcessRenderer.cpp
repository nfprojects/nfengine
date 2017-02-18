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

using namespace Math;

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

    mTonemappingPipelineState.Load("Tonemapping");

    CreateResourceBindingLayouts();

    /// create vertex layout
    VertexLayoutElement vertexLayoutElements[] =
    {
        { ElementFormat::R32G32B32_Float, 0, 0, false, 0 }, // position
    };
    VertexLayoutDesc vertexLayoutDesc;
    vertexLayoutDesc.elements = vertexLayoutElements;
    vertexLayoutDesc.numElements = 1;
    vertexLayoutDesc.debugName = "PostProcessRenderer::mVertexLayout";
    mVertexLayout= device->CreateVertexLayout(vertexLayoutDesc);

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
    bufferDesc.mode = BufferMode::Static;
    bufferDesc.size = sizeof(vertices);
    bufferDesc.type = BufferType::Vertex;
    bufferDesc.initialData = vertices;
    bufferDesc.debugName = "PostProcessRenderer::mVertexBuffer";
    mVertexBuffer= device->CreateBuffer(bufferDesc);

    bufferDesc.mode = BufferMode::Volatile;
    bufferDesc.type = BufferType::Constant;
    bufferDesc.initialData = nullptr;
    bufferDesc.size = sizeof(ToneMappingCBuffer);
    bufferDesc.debugName = "PostProcessRenderer::mTonemappingCBuffer";
    mTonemappingCBuffer= device->CreateBuffer(bufferDesc);

    mNullTextureBindingInstance = CreateTextureBinding(mRenderer->GetDefaultDiffuseTexture());

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.resBindingLayout = mResBindingLayout;
    pipelineStateDesc.raterizerState.cullMode = CullMode::Disabled;
    pipelineStateDesc.raterizerState.fillMode = FillMode::Solid;
    pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
    pipelineStateDesc.vertexLayout = mVertexLayout;
    pipelineStateDesc.debugName = "PostProcessRenderer::mTonemappingPipelineState";
    pipelineStateDesc.numRenderTargets = 1;
    pipelineStateDesc.rtFormats[0] = mRenderer->GetBackbufferFormat();
    mTonemappingPipelineState.Build(pipelineStateDesc);
}

bool PostProcessRenderer::CreateResourceBindingLayouts()
{
    IDevice* device = mRenderer->GetDevice();

    int paramsCBufferSlot = mTonemappingPipelineState.GetResourceSlotByName("Params");
    if (paramsCBufferSlot < 0)
        return false;

    int sourceTextureSlot = mTonemappingPipelineState.GetResourceSlotByName("gSourceTexture");
    if (sourceTextureSlot < 0)
        return false;

    VolatileCBufferBinding cbufferBindingDesc(ShaderType::Pixel, ShaderResourceType::CBuffer, paramsCBufferSlot);

    std::vector<ResourceBindingSetPtr> bindingSets;
    ResourceBindingDesc binding(ShaderResourceType::Texture, sourceTextureSlot,
                                 mRenderer->GetDefaultSampler());
    mTexturesBindingSet = device->CreateResourceBindingSet(ResourceBindingSetDesc(&binding, 1, ShaderType::Pixel));
    if (!mTexturesBindingSet)
        return false;
    bindingSets.push_back(mTexturesBindingSet);

    // create binding layout
    mResBindingLayout = device->CreateResourceBindingLayout(
        ResourceBindingLayoutDesc(bindingSets.data(), bindingSets.size(), &cbufferBindingDesc, 1));
    if (!mResBindingLayout)
        return false;

    return true;
}

ResourceBindingInstancePtr PostProcessRenderer::CreateTextureBinding(const TexturePtr& texture)
{
    IDevice* device = mRenderer->GetDevice();

    ResourceBindingInstancePtr bindingInstance(
        device->CreateResourceBindingInstance(mTexturesBindingSet));
    if (!bindingInstance)
        return ResourceBindingInstancePtr();
    if (!bindingInstance->WriteTextureView(0, texture))
        return ResourceBindingInstancePtr();

    return bindingInstance;
}

void PostProcessRenderer::OnEnter(PostProcessRendererContext* context)
{
    context->commandRecorder->BeginDebugGroup("Post Process Renderer stage");

    context->commandRecorder->SetResourceBindingLayout(mResBindingLayout);

    BufferPtr veretexBuffers[] = { mVertexBuffer };
    int strides[] = { sizeof(Float3) };
    int offsets[] = { 0 };
    context->commandRecorder->SetVertexBuffers(1, veretexBuffers, strides, offsets);
}

void PostProcessRenderer::OnLeave(PostProcessRendererContext* context)
{
    context->commandRecorder->EndDebugGroup();
}

void PostProcessRenderer::ApplyTonemapping(PostProcessRendererContext* context,
                                           const ToneMappingParameters& params,
                                           ResourceBindingInstancePtr src, RenderTargetPtr dest)
{
    int width, height;
    dest->GetDimensions(width, height);
    context->commandRecorder->SetViewport(0.0f, static_cast<float>(width),
                                        0.0f, static_cast<float>(height),
                                        0.0f, 1.0f);
    context->commandRecorder->SetScissors(0, 0, width, height);

    context->commandRecorder->SetPipelineState(mTonemappingPipelineState.GetPipelineState());

    ToneMappingCBuffer cbufferData;
    cbufferData.bufferInvRes = Vector(1.0f / static_cast<float>(width),
                                      1.0f / static_cast<float>(height));
    cbufferData.params = Vector(params.saturation,
                                params.noiseFactor,
                                expf(params.exposureOffset));
    cbufferData.seed = Vector(context->random.GetFloat2());

    context->commandRecorder->BindResources(0, src);
    context->commandRecorder->BindVolatileCBuffer(0, mTonemappingCBuffer);
    context->commandRecorder->WriteBuffer(mTonemappingCBuffer, 0,
                                        sizeof(cbufferData), &cbufferData);

    context->commandRecorder->SetRenderTarget(dest);


    // TODO: use compute shaders if supported
    context->commandRecorder->Draw(2 * 3);  // draw 2 triangles

    // unbind source texture
    context->commandRecorder->BindResources(0, mNullTextureBindingInstance);
}

} // namespace Renderer
} // namespace NFE
