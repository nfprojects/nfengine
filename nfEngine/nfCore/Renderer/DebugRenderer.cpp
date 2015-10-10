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
#include "../../nfCommon/Logger.hpp"

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
    ShaderProgramDesc shaderProgDesc;

    mVertexShader.Load("DebugVS");
    mIsMeshMacroId = mPixelShader.GetMacroByName("IS_MESH");

    mPixelShader.Load("DebugPS");
    mUseTextureMacroId = mPixelShader.GetMacroByName("USE_TEXTURE");

    /// create vertex layout
    VertexLayoutElement vertexLayoutElements[] =
    {
        { ElementFormat::Float_32,    3, 0,  0, false, 0 }, // position
        { ElementFormat::Uint_8_norm, 4, 12, 0, false, 0 }, // color
        { ElementFormat::Float_32,    2, 16, 0, false, 0 }, // tex-coords
    };
    int isMesh = 0; // IS_MESH macro
    VertexLayoutDesc vertexLayoutDesc;
    vertexLayoutDesc.elements = vertexLayoutElements;
    vertexLayoutDesc.numElements = 3;
    vertexLayoutDesc.vertexShader = mVertexShader.GetShader(&isMesh);
    vertexLayoutDesc.debugName = "DebugRenderer::mVertexLayout";
    mVertexLayout.reset(device->CreateVertexLayout(vertexLayoutDesc));

    /// create vertex layout for meshes
    VertexLayoutElement meshVertexLayoutElements[] =
    {
        { ElementFormat::Float_32,   3, 0,  0, false, 0 }, // position
        { ElementFormat::Float_32,   2, 12, 0, false, 0 }, // tex-coords
        { ElementFormat::Int_8_norm, 4, 20, 0, false, 0 }, // normal
        { ElementFormat::Int_8_norm, 4, 24, 0, false, 0 }, // tangent
    };
    isMesh = 1;
    VertexLayoutDesc meshVertexLayoutDesc;
    meshVertexLayoutDesc.elements = meshVertexLayoutElements;
    meshVertexLayoutDesc.numElements = 4;
    meshVertexLayoutDesc.vertexShader = mVertexShader.GetShader(&isMesh);
    meshVertexLayoutDesc.debugName = "DebugRenderer::mMeshVertexLayout";
    mMeshVertexLayout.reset(device->CreateVertexLayout(meshVertexLayoutDesc));

    /// create constant buffer
    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = sizeof(DebugCBuffer);
    bufferDesc.type = BufferType::Constant;
    bufferDesc.debugName = "DebugRenderer::mConstantBuffer";
    mConstantBuffer.reset(device->CreateBuffer(bufferDesc));

    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = sizeof(DebugPerMeshCBuffer);
    bufferDesc.type = BufferType::Constant;
    bufferDesc.debugName = "DebugRenderer::mPerMeshConstantBuffer";
    mPerMeshConstantBuffer.reset(device->CreateBuffer(bufferDesc));

    /// create vertex buffer
    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = gVertexBufferSize * sizeof(DebugVertex);
    bufferDesc.type = BufferType::Vertex;
    bufferDesc.debugName = "DebugRenderer::mVertexBuffer";
    mVertexBuffer.reset(device->CreateBuffer(bufferDesc));

    /// create index buffer
    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = gIndexBufferSize * sizeof(DebugIndexType);
    bufferDesc.type = BufferType::Index;
    bufferDesc.debugName = "DebugRenderer::mIndexBuffer";
    mIndexBuffer.reset(device->CreateBuffer(bufferDesc));

    /// create rasterizer state
    RasterizerStateDesc rasterizerStateDesc;
    rasterizerStateDesc.cullMode = CullMode::Disabled;
    rasterizerStateDesc.fillMode = FillMode::Solid;
    rasterizerStateDesc.debugName = "DebugRenderer::mRasterizerState";
    mRasterizerState.reset(device->CreateRasterizerState(rasterizerStateDesc));

    // TODO: depth state
}

void DebugRenderer::OnEnter(RenderContext* context)
{
    context->debugContext.mode = DebugRendererMode::Unknown;

    context->commandBuffer->BeginDebugGroup("Debug Renderer stage");

    int psMacros[] = { 0 }; // USE_TEXTURE
    context->commandBuffer->SetShader(mPixelShader.GetShader(psMacros));

    IBuffer* constantBuffers[] = { mConstantBuffer.get(), mPerMeshConstantBuffer.get() };
    context->commandBuffer->SetConstantBuffers(constantBuffers, 2, ShaderType::Vertex);

    context->commandBuffer->SetRasterizerState(mRasterizerState.get());
    context->commandBuffer->SetDepthState(mRenderer->GetDefaultDepthState());

    ISampler* sampler = mRenderer->GetDefaultSampler();
    context->commandBuffer->SetSamplers(&sampler, 1, ShaderType::Pixel);
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

        int vsMacros[] = { 0 }; // IS_MESH
        context->commandBuffer->SetShader(mVertexShader.GetShader(vsMacros));
        context->commandBuffer->SetVertexLayout(mVertexLayout.get());

        ctx.mode = DebugRendererMode::Simple;
    }

    context->commandBuffer->WriteBuffer(mVertexBuffer.get(), 0,
                                        ctx.queuedVertices * sizeof(DebugVertex),
                                        ctx.vertices.get());

    context->commandBuffer->WriteBuffer(mIndexBuffer.get(), 0,
                                        ctx.queuedIndicies * sizeof(DebugIndexType),
                                        ctx.indicies.get());

    context->commandBuffer->DrawIndexed(PrimitiveType::Lines,
                                        static_cast<int>(ctx.queuedIndicies));

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
    ITexture* tex = nullptr;

    if (material && material->mLayers)
        if (material->mLayers[0].diffuseTexture)
            tex = material->mLayers[0].diffuseTexture->GetRendererTexture();

    int macros[1] = { tex != nullptr ? 1 : 0 };
    context->commandBuffer->SetShader(mPixelShader.GetShader(macros));

    if (tex)
        context->commandBuffer->SetTextures(&tex, 1, ShaderType::Pixel);
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
        int vsMacros[] = { 1 }; // IS_MESH
        context->commandBuffer->SetShader(mVertexShader.GetShader(vsMacros));
        context->commandBuffer->SetVertexLayout(mMeshVertexLayout.get());

        ctx.mode = DebugRendererMode::Meshes;
        ctx.polyType = PrimitiveType::Unknown;
    }

    /// update model matrix
    DebugPerMeshCBuffer perMeshCBuffer;
    perMeshCBuffer.modelMatrix = matrix;
    context->commandBuffer->WriteBuffer(mPerMeshConstantBuffer.get(), 0,
                                        sizeof(DebugPerMeshCBuffer),
                                        &perMeshCBuffer);

    int strides[] = { sizeof(MeshVertex) };
    int offsets[] = { 0 };
    context->commandBuffer->SetVertexBuffers(1, &vb, strides, offsets);
    context->commandBuffer->SetIndexBuffer(ib, IndexBufferFormat::Uint32);
    for (uint32 i = 0; i < mesh->mSubMeshesCount; ++i)
    {
        const Resource::SubMesh& subMesh = mesh->mSubMeshes[i];
        SetMeshMaterial(context, subMesh.material);
        context->commandBuffer->DrawIndexed(PrimitiveType::Triangles, 3 * subMesh.trianglesCount, 1,
                                            subMesh.indexOffset);
    }
}

} // namespace Renderer
} // namespace NFE
