/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of high-level Debug Renderer
 */

#pragma once

#include "PCH.hpp"
#include "DebugRenderer.hpp"
#include "HighLevelRenderer.hpp"
#include "Resources/Material.hpp"
#include "Resources/Mesh.hpp"
#include "nfCommon/Logger.hpp"

namespace NFE {
namespace Renderer {

// renderer modules instance definition
std::unique_ptr<DebugRenderer> DebugRenderer::mPtr;


/**
 * Debug Renderer constant buffer layout for vertex shader.
 */
struct NFE_ALIGN16 DebugCBuffer
{
    Matrix viewMatrix;
    Matrix projMatrix;
};

struct NFE_ALIGN16 DebugPerMeshCBuffer
{
    Matrix modelMatrix;
};

namespace {

/*
 * Size of vertex and index buffer for accumulating geometry data from multiple
 * DebugRenderer::Draw...() calls.
 */
const size_t gVertexBufferSize = 4096;
const size_t gIndexBufferSize = 8192;

// box frame indicies
const DebugIndexType gBoxIndexBuffer[] =
{
    0, 1, 1, 3, 3, 2, 2, 0,   // front side
    4, 5, 5, 7, 7, 6, 6, 4,   // back side
    0, 4, 1, 5, 2, 6, 3, 7,   // fron-back connections
};

} // namespace


DebugRendererContext::DebugRendererContext()
{
    /// allocate buffers for verticies and indicies
    vertices.reset(new DebugVertex[gVertexBufferSize]);
    indicies.reset(new DebugIndexType[gIndexBufferSize]);
    queuedVertices = 0;
    queuedIndicies = 0;
    polyType = PrimitiveType::Lines;
    mode = DebugRendererMode::Unknown;
}

DebugRenderer::DebugRenderer()
{
    IDevice* device = mRenderer->GetDevice();

    BufferDesc bufferDesc;
    ShaderDesc shaderDesc;

    mLinesPipelineState.Load("Debug");
    mTrianglesPipelineState.Load("Debug");
    mMeshPipelineState.Load("Debug");

    // TODO
    mIsMeshMacroId = mLinesPipelineState.GetMacroByName("IS_MESH");
    mUseTextureMacroId = mLinesPipelineState.GetMacroByName("USE_TEXTURE");

    CreateResourceBindingLayouts();

    /// create vertex layout
    VertexLayoutElement vertexLayoutElements[] =
    {
        { ElementFormat::R32G32B32_Float,   0,  0, false, 0 }, // position
        { ElementFormat::R8G8B8A8_U_Norm,   12, 0, false, 0 }, // color
        { ElementFormat::R32G32_Float,      16, 0, false, 0 }, // tex-coords
    };
    VertexLayoutDesc vertexLayoutDesc;
    vertexLayoutDesc.elements = vertexLayoutElements;
    vertexLayoutDesc.numElements = 3;
    vertexLayoutDesc.debugName = "DebugRenderer::mVertexLayout";
    mVertexLayout.reset(device->CreateVertexLayout(vertexLayoutDesc));

    /// create vertex layout for meshes
    VertexLayoutElement meshVertexLayoutElements[] =
    {
        { ElementFormat::R32G32B32_Float,   0,  0, false, 0 }, // position
        { ElementFormat::R32G32_Float,      12, 0, false, 0 }, // tex-coords
        { ElementFormat::R8G8B8A8_S_Norm,   20, 0, false, 0 }, // normal
        { ElementFormat::R8G8B8A8_S_Norm,   24, 0, false, 0 }, // tangent
    };
    VertexLayoutDesc meshVertexLayoutDesc;
    meshVertexLayoutDesc.elements = meshVertexLayoutElements;
    meshVertexLayoutDesc.numElements = 4;
    meshVertexLayoutDesc.debugName = "DebugRenderer::mMeshVertexLayout";
    mMeshVertexLayout.reset(device->CreateVertexLayout(meshVertexLayoutDesc));

    /// create constant buffer
    bufferDesc.mode = BufferMode::Dynamic;
    bufferDesc.size = sizeof(DebugCBuffer);
    bufferDesc.type = BufferType::Constant;
    bufferDesc.debugName = "DebugRenderer::mConstantBuffer";
    mConstantBuffer.reset(device->CreateBuffer(bufferDesc));

    bufferDesc.mode = BufferMode::Volatile;
    bufferDesc.size = sizeof(DebugPerMeshCBuffer);
    bufferDesc.type = BufferType::Constant;
    bufferDesc.debugName = "DebugRenderer::mPerMeshConstantBuffer";
    mPerMeshConstantBuffer.reset(device->CreateBuffer(bufferDesc));

    mVertexShaderBindingInstance.reset(
        device->CreateResourceBindingInstance(mVertexShaderBindingSet.get()));
    if (mVertexShaderBindingInstance)
    {
        mVertexShaderBindingInstance->WriteCBufferView(0, mConstantBuffer.get());
        mVertexShaderBindingInstance->WriteCBufferView(1, mPerMeshConstantBuffer.get());
    }


    /// create vertex buffer
    bufferDesc.mode = BufferMode::Dynamic;
    bufferDesc.size = gVertexBufferSize * sizeof(DebugVertex);
    bufferDesc.type = BufferType::Vertex;
    bufferDesc.debugName = "DebugRenderer::mVertexBuffer";
    mVertexBuffer.reset(device->CreateBuffer(bufferDesc));

    /// create index buffer
    bufferDesc.mode = BufferMode::Dynamic;
    bufferDesc.size = gIndexBufferSize * sizeof(DebugIndexType);
    bufferDesc.type = BufferType::Index;
    bufferDesc.debugName = "DebugRenderer::mIndexBuffer";
    mIndexBuffer.reset(device->CreateBuffer(bufferDesc));

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.resBindingLayout = mResBindingLayout.get();
    pipelineStateDesc.primitiveType = PrimitiveType::Lines;
    pipelineStateDesc.vertexLayout = mVertexLayout.get();
    pipelineStateDesc.raterizerState.cullMode = CullMode::Disabled;
    pipelineStateDesc.raterizerState.fillMode = FillMode::Solid;
    pipelineStateDesc.debugName = "DebugRenderer::mLinesPipelineState";
    mLinesPipelineState.Build(pipelineStateDesc);

    pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
    pipelineStateDesc.debugName = "DebugRenderer::mTrianglesPipelineState";
    mTrianglesPipelineState.Build(pipelineStateDesc);

    pipelineStateDesc.vertexLayout = mMeshVertexLayout.get();
    pipelineStateDesc.debugName = "DebugRenderer::mMeshPipelineState";
    mMeshPipelineState.Build(pipelineStateDesc);

    // TODO: depth state
}

bool DebugRenderer::CreateResourceBindingLayouts()
{
    IDevice* device = mRenderer->GetDevice();

    int globalCBufferSlot = mMeshPipelineState.GetResourceSlotByName("Global");
    if (globalCBufferSlot < 0)
        return false;

    int perMeshCBufferSlot = mMeshPipelineState.GetResourceSlotByName("PerMesh");
    if (perMeshCBufferSlot < 0)
        return false;

    ResourceBindingDesc gbufferBindings[2] =
    {
        ResourceBindingDesc(ShaderResourceType::CBuffer, globalCBufferSlot),
        ResourceBindingDesc(ShaderResourceType::CBuffer, perMeshCBufferSlot),
    };
    mVertexShaderBindingSet.reset(device->CreateResourceBindingSet(
        ResourceBindingSetDesc(gbufferBindings, 2, ShaderType::Vertex)));
    if (!mVertexShaderBindingSet)
        return false;
    // TODO: material binding set (textures)

    // create binding layout
    IResourceBindingSet* sets[] = { mVertexShaderBindingSet.get() };
    mResBindingLayout.reset(device->CreateResourceBindingLayout(
        ResourceBindingLayoutDesc(sets, 1)));
    if (!mResBindingLayout)
        return false;

    return true;
}

void DebugRenderer::OnEnter(RenderContext* context)
{
    context->debugContext.mode = DebugRendererMode::Unknown;

    context->commandBuffer->BeginDebugGroup("Debug Renderer stage");
}

void DebugRenderer::OnLeave(RenderContext* context)
{
    Flush(context);
    context->commandBuffer->EndDebugGroup();
}

void DebugRenderer::Flush(RenderContext* context)
{
    DebugRendererContext& ctx = context->debugContext;

    if (ctx.queuedVertices == 0 && ctx.queuedIndicies == 0)
        return;

    if (ctx.mode != DebugRendererMode::Simple)
    {
        int stride = sizeof(DebugVertex);
        int offset = 0;
        IBuffer* vb = mVertexBuffer.get();
        context->commandBuffer->SetVertexBuffers(1, &vb, &stride, &offset);
        context->commandBuffer->SetIndexBuffer(mIndexBuffer.get(), IndexBufferFormat::Uint16);

        int macros[] = { 0, 0 }; // IS_MESH

        if (ctx.polyType == PrimitiveType::Lines)
            context->commandBuffer->SetPipelineState(mLinesPipelineState.GetPipelineState(macros));
        else
            context->commandBuffer->SetPipelineState(mTrianglesPipelineState.GetPipelineState(macros));

        context->commandBuffer->BindResources(0, mVertexShaderBindingInstance.get());

        ctx.mode = DebugRendererMode::Simple;
    }

    context->commandBuffer->WriteBuffer(mVertexBuffer.get(), 0,
                                        ctx.queuedVertices * sizeof(DebugVertex),
                                        ctx.vertices.get());

    context->commandBuffer->WriteBuffer(mIndexBuffer.get(), 0,
                                        ctx.queuedIndicies * sizeof(DebugIndexType),
                                        ctx.indicies.get());

    context->commandBuffer->DrawIndexed(static_cast<int>(ctx.queuedIndicies));

    ctx.queuedVertices = 0;
    ctx.queuedIndicies = 0;
}

void DebugRenderer::SetTarget(RenderContext *context, IRenderTarget* target)
{
    if (target)
    {
        int width, height;
        target->GetDimensions(width, height);

        context->commandBuffer->SetRenderTarget(target);
        context->commandBuffer->SetViewport(0.0f, static_cast<float>(width),
                                            0.0f, static_cast<float>(height),
                                            0.0f, 1.0f);
    }
}

void DebugRenderer::SetCamera(RenderContext *context, const Matrix& viewMatrix,
                              const Matrix& projMatrix)
{
    DebugCBuffer debugCBufferData;
    debugCBufferData.projMatrix = projMatrix;
    debugCBufferData.viewMatrix = viewMatrix;
    context->commandBuffer->WriteBuffer(mConstantBuffer.get(), 0, sizeof(DebugCBuffer),
                                        &debugCBufferData);
}

void DebugRenderer::DrawLine(RenderContext *context, const Vector& A, const Vector& B,
                             const uint32 color)
{
    DebugRendererContext& ctx = context->debugContext;

    // check if the shape will fit into the buffers
    if ((ctx.queuedVertices >= gVertexBufferSize - 2) ||
        (ctx.queuedIndicies >= gIndexBufferSize - 2) ||
        (ctx.polyType != PrimitiveType::Lines))
    {
        Flush(context);
        ctx.polyType = PrimitiveType::Lines;
    }

    DebugVertex vert;
    vert.color = color;

    VectorStore(A, &vert.pos);
    ctx.indicies[ctx.queuedIndicies++] = static_cast<DebugIndexType>(ctx.queuedVertices);
    ctx.vertices[ctx.queuedVertices++] = vert;

    VectorStore(B, &vert.pos);
    ctx.indicies[ctx.queuedIndicies++] = static_cast<DebugIndexType>(ctx.queuedVertices);
    ctx.vertices[ctx.queuedVertices++] = vert;
}

void DebugRenderer::DrawLine(RenderContext *context, const Float3& A, const Float3& B,
                             const uint32 color)
{
    DebugRendererContext& ctx = context->debugContext;

    // check if the shape will fit into the buffers
    if ((ctx.queuedVertices >= gVertexBufferSize - 2) ||
        (ctx.queuedIndicies >= gIndexBufferSize - 2) ||
        (ctx.polyType != PrimitiveType::Lines))
    {
        Flush(context);
        ctx.polyType = PrimitiveType::Lines;
    }

    DebugVertex vert;
    vert.color = color;

    vert.pos = A;
    ctx.indicies[ctx.queuedIndicies++] = static_cast<DebugIndexType>(ctx.queuedVertices);
    ctx.vertices[ctx.queuedVertices++] = vert;

    vert.pos = B;
    ctx.indicies[ctx.queuedIndicies++] = static_cast<DebugIndexType>(ctx.queuedVertices);
    ctx.vertices[ctx.queuedVertices++] = vert;
}

void DebugRenderer::DrawBox(RenderContext *context, const Box& box, const uint32 color)
{
    const size_t vertexRequired = 8;
    const size_t indexRequired = 24;

    DebugRendererContext& ctx = context->debugContext;

    // check if the shape will fit into the buffers
    if ((ctx.queuedVertices >= gVertexBufferSize - vertexRequired) ||
        (ctx.queuedIndicies >= gIndexBufferSize - indexRequired) ||
        (ctx.polyType != PrimitiveType::Lines))
    {
        Flush(context);
        ctx.polyType = PrimitiveType::Lines;
    }

    DebugVertex* boxVerticies = ctx.vertices.get() + ctx.queuedVertices;
    for (size_t i = 0; i < vertexRequired; ++i)
    {
        Vector v = box.GetVertex(static_cast<int>(i));
        VectorStore(v, &boxVerticies[i].pos);
        boxVerticies[i].color = color;
    }

    DebugIndexType* boxIndicies = ctx.indicies.get() + ctx.queuedIndicies;
    for (int i = 0; i < indexRequired; i++)
        boxIndicies[i] = gBoxIndexBuffer[i] + static_cast<DebugIndexType>(ctx.queuedVertices);

    ctx.queuedVertices += vertexRequired;
    ctx.queuedIndicies += indexRequired;
}

void DebugRenderer::DrawFilledBox(RenderContext *context, const Box& box, const uint32 color)
{
    // TODO
}

void DebugRenderer::DrawFrustum(RenderContext *context, const Frustum& frustum, const uint32 color)
{
    const size_t vertexRequired = 8;
    const size_t indexRequired = 24;

    DebugRendererContext& ctx = context->debugContext;

    // check if the shape will fit into the buffers
    if ((ctx.queuedVertices >= gVertexBufferSize - vertexRequired) ||
        (ctx.queuedIndicies >= gIndexBufferSize - indexRequired) ||
        (ctx.polyType != PrimitiveType::Lines))
    {
        Flush(context);
        ctx.polyType = PrimitiveType::Lines;
    }

    DebugVertex* boxVerticies = ctx.vertices.get() + ctx.queuedVertices;
    for (size_t i = 0; i < vertexRequired; ++i)
    {
        Vector v = frustum.verticies[i];
        VectorStore(v, &boxVerticies[i].pos);
        boxVerticies[i].color = color;
    }

    DebugIndexType* boxIndicies = ctx.indicies.get() + ctx.queuedIndicies;
    for (int i = 0; i < indexRequired; i++)
        boxIndicies[i] = gBoxIndexBuffer[i] + static_cast<DebugIndexType>(ctx.queuedVertices);

    ctx.queuedVertices += vertexRequired;
    ctx.queuedIndicies += indexRequired;
}

void DebugRenderer::SetMeshMaterial(RenderContext* context, const Resource::Material* material)
{
    // ITexture* tex = nullptr;

    // FIXME
    /*
    if (material && material->mLayers)
        if (material->mLayers[0].diffuseTexture)
            tex = material->mLayers[0].diffuseTexture->GetRendererTexture();
            */

    int macros[2];
    macros[mIsMeshMacroId] = 1;

    // FIXME
    macros[mUseTextureMacroId] = 0;
    // macros[mUseTextureMacroId] = tex != nullptr ? 1 : 0;

    context->commandBuffer->SetPipelineState(mMeshPipelineState.GetPipelineState(macros));

    // FIXME
    // if (tex)
    //    context->commandBuffer->SetTextures(&tex, 1, ShaderType::Pixel);ale n
}

void DebugRenderer::DrawMesh(RenderContext* context, const Resource::Mesh* mesh,
                             const Matrix& matrix)
{
    if (mesh->GetState() != Resource::ResourceState::Loaded)
        return;

    Flush(context);

    DebugRendererContext& ctx = context->debugContext;
    IBuffer* vb = mesh->mVB.get();
    IBuffer* ib = mesh->mIB.get();

    if (vb == nullptr || ib == nullptr)
    {
        LOG_ERROR("Invalid vertex or index buffer");
        return;
    }

    if (ctx.mode != DebugRendererMode::Meshes)
    {
        context->commandBuffer->BindResources(0, mVertexShaderBindingInstance.get());
        ctx.mode = DebugRendererMode::Meshes;
        ctx.polyType = PrimitiveType::Unknown;
    }

    /// update model matrix
    DebugPerMeshCBuffer perMeshCBuffer;
    perMeshCBuffer.modelMatrix = matrix;
    context->commandBuffer->WriteBuffer(mPerMeshConstantBuffer.get(), 0,
                                        sizeof(DebugPerMeshCBuffer),
                                        &perMeshCBuffer);

    int strides[] = { sizeof(Resource::MeshVertex) };
    int offsets[] = { 0 };
    context->commandBuffer->SetVertexBuffers(1, &vb, strides, offsets);
    context->commandBuffer->SetIndexBuffer(ib, IndexBufferFormat::Uint32);
    for (const auto& subMesh : mesh->mSubMeshes)
    {
        SetMeshMaterial(context, subMesh.material);
        context->commandBuffer->DrawIndexed(3 * subMesh.trianglesCount, 1, subMesh.indexOffset);
    }
}

} // namespace Renderer
} // namespace NFE
