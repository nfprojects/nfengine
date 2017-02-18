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

// box frame indicies
const DebugIndexType gBoxIndexBuffer[] =
{
    0, 1, 1, 3, 3, 2, 2, 0,   // front side
    4, 5, 5, 7, 7, 6, 6, 4,   // back side
    0, 4, 1, 5, 2, 6, 3, 7,   // fron-back connections
};

} // namespace

  /*
  * Size of vertex and index buffer for accumulating geometry data from multiple
  * DebugRenderer::Draw...() calls.
  */
const size_t DebugRendererContext::gVertexBufferSize = 4096;
const size_t DebugRendererContext::gIndexBufferSize = 8192;


DebugRenderer::DebugRenderer()
{
    IDevice* device = mRenderer->GetDevice();

    BufferDesc bufferDesc;
    ShaderDesc shaderDesc;

    mLinesPipelineState.Load("Debug");
    mTrianglesPipelineState.Load("Debug");
    mMeshPipelineState.Load("DebugMesh");

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

    /// create vertex buffer
    bufferDesc.mode = BufferMode::Dynamic;
    bufferDesc.size = DebugRendererContext::gVertexBufferSize * sizeof(DebugVertex);
    bufferDesc.type = BufferType::Vertex;
    bufferDesc.debugName = "DebugRenderer::mVertexBuffer";
    mVertexBuffer.reset(device->CreateBuffer(bufferDesc));

    /// create index buffer
    bufferDesc.mode = BufferMode::Dynamic;
    bufferDesc.size = DebugRendererContext::gIndexBufferSize * sizeof(DebugIndexType);
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

    VolatileCBufferBinding cbufferBindingsDesc[2] =
    {
        VolatileCBufferBinding(ShaderType::Vertex, ShaderResourceType::CBuffer, globalCBufferSlot),
        VolatileCBufferBinding(ShaderType::Vertex, ShaderResourceType::CBuffer, perMeshCBufferSlot),
    };

    // TODO: material binding set (textures)

    // create binding layout
    mResBindingLayout.reset(device->CreateResourceBindingLayout(
        ResourceBindingLayoutDesc(nullptr, 0, cbufferBindingsDesc, 2)));
    if (!mResBindingLayout)
        return false;

    return true;
}

void DebugRenderer::OnEnter(DebugRendererContext* context)
{
    context->mode = DebugRendererMode::Unknown;
    context->commandRecorder->BeginDebugGroup("Debug Renderer stage");
}

void DebugRenderer::OnLeave(DebugRendererContext* context)
{
    Flush(context);
    context->commandRecorder->EndDebugGroup();
}

void DebugRenderer::Flush(DebugRendererContext* context)
{
    if (context->queuedVertices == 0 && context->queuedIndicies == 0)
        return;

    if (context->mode != DebugRendererMode::Simple)
    {
        int stride = sizeof(DebugVertex);
        int offset = 0;
        IBuffer* vb = mVertexBuffer.get();
        context->commandRecorder->SetVertexBuffers(1, &vb, &stride, &offset);
        context->commandRecorder->SetIndexBuffer(mIndexBuffer.get(), IndexBufferFormat::Uint16);

        int macros[] = { 0, 0 }; // IS_MESH

        if (context->polyType == PrimitiveType::Lines)
            context->commandRecorder->SetPipelineState(mLinesPipelineState.GetPipelineState(macros));
        else
            context->commandRecorder->SetPipelineState(mTrianglesPipelineState.GetPipelineState(macros));

        context->commandRecorder->BindVolatileCBuffer(0, mConstantBuffer.get());
        context->commandRecorder->BindVolatileCBuffer(1, mPerMeshConstantBuffer.get());

        context->mode = DebugRendererMode::Simple;
    }

    context->commandRecorder->WriteBuffer(mVertexBuffer.get(), 0,
                                        context->queuedVertices * sizeof(DebugVertex),
                                        context->vertices.get());

    context->commandRecorder->WriteBuffer(mIndexBuffer.get(), 0,
                                        context->queuedIndicies * sizeof(DebugIndexType),
                                        context->indicies.get());

    context->commandRecorder->DrawIndexed(static_cast<int>(context->queuedIndicies));

    context->queuedVertices = 0;
    context->queuedIndicies = 0;
}

void DebugRenderer::SetTarget(DebugRendererContext *context, IRenderTarget* target)
{
    if (target)
    {
        int width, height;
        target->GetDimensions(width, height);

        context->commandRecorder->SetRenderTarget(target);
        context->commandRecorder->SetViewport(0.0f, static_cast<float>(width),
                                            0.0f, static_cast<float>(height),
                                            0.0f, 1.0f);
        context->commandRecorder->SetScissors(0, 0, width, height);
    }
}

void DebugRenderer::SetCamera(DebugRendererContext *context, const Matrix& viewMatrix,
                              const Matrix& projMatrix)
{
    DebugCBuffer debugCBufferData;
    debugCBufferData.projMatrix = projMatrix;
    debugCBufferData.viewMatrix = viewMatrix;
    context->commandRecorder->WriteBuffer(mConstantBuffer.get(), 0, sizeof(DebugCBuffer),
                                        &debugCBufferData);
}

void DebugRenderer::DrawLine(DebugRendererContext *context, const Vector& A, const Vector& B,
                             const uint32 color)
{
    // check if the shape will fit into the buffers
    if ((context->queuedVertices >= DebugRendererContext::gVertexBufferSize - 2) ||
        (context->queuedIndicies >= DebugRendererContext::gIndexBufferSize - 2) ||
        (context->polyType != PrimitiveType::Lines))
    {
        Flush(context);
        context->polyType = PrimitiveType::Lines;
    }

    DebugVertex vert;
    vert.color = color;

    VectorStore(A, &vert.pos);
    context->indicies[context->queuedIndicies++] = static_cast<DebugIndexType>(context->queuedVertices);
    context->vertices[context->queuedVertices++] = vert;

    VectorStore(B, &vert.pos);
    context->indicies[context->queuedIndicies++] = static_cast<DebugIndexType>(context->queuedVertices);
    context->vertices[context->queuedVertices++] = vert;
}

void DebugRenderer::DrawLine(DebugRendererContext *context, const Float3& A, const Float3& B,
                             const uint32 color)
{
    // check if the shape will fit into the buffers
    if ((context->queuedVertices >= DebugRendererContext::gVertexBufferSize - 2) ||
        (context->queuedIndicies >= DebugRendererContext::gIndexBufferSize - 2) ||
        (context->polyType != PrimitiveType::Lines))
    {
        Flush(context);
        context->polyType = PrimitiveType::Lines;
    }

    DebugVertex vert;
    vert.color = color;

    vert.pos = A;
    context->indicies[context->queuedIndicies++] = static_cast<DebugIndexType>(context->queuedVertices);
    context->vertices[context->queuedVertices++] = vert;

    vert.pos = B;
    context->indicies[context->queuedIndicies++] = static_cast<DebugIndexType>(context->queuedVertices);
    context->vertices[context->queuedVertices++] = vert;
}

void DebugRenderer::DrawBox(DebugRendererContext *context, const Box& box, const uint32 color)
{
    const size_t vertexRequired = 8;
    const size_t indexRequired = 24;

    // check if the shape will fit into the buffers
    if ((context->queuedVertices >= DebugRendererContext::gVertexBufferSize - vertexRequired) ||
        (context->queuedIndicies >= DebugRendererContext::gIndexBufferSize - indexRequired) ||
        (context->polyType != PrimitiveType::Lines))
    {
        Flush(context);
        context->polyType = PrimitiveType::Lines;
    }

    DebugVertex* boxVerticies = context->vertices.get() + context->queuedVertices;
    for (size_t i = 0; i < vertexRequired; ++i)
    {
        Vector v = box.GetVertex(static_cast<int>(i));
        VectorStore(v, &boxVerticies[i].pos);
        boxVerticies[i].color = color;
    }

    DebugIndexType* boxIndicies = context->indicies.get() + context->queuedIndicies;
    for (int i = 0; i < indexRequired; i++)
        boxIndicies[i] = gBoxIndexBuffer[i] + static_cast<DebugIndexType>(context->queuedVertices);

    context->queuedVertices += vertexRequired;
    context->queuedIndicies += indexRequired;
}

void DebugRenderer::DrawFilledBox(DebugRendererContext *context, const Box& box, const uint32 color)
{
    // TODO
}

void DebugRenderer::DrawFrustum(DebugRendererContext *context, const Frustum& frustum, const uint32 color)
{
    const size_t vertexRequired = 8;
    const size_t indexRequired = 24;

    // check if the shape will fit into the buffers
    if ((context->queuedVertices >= DebugRendererContext::gVertexBufferSize - vertexRequired) ||
        (context->queuedIndicies >= DebugRendererContext::gIndexBufferSize - indexRequired) ||
        (context->polyType != PrimitiveType::Lines))
    {
        Flush(context);
        context->polyType = PrimitiveType::Lines;
    }

    DebugVertex* boxVerticies = context->vertices.get() + context->queuedVertices;
    for (size_t i = 0; i < vertexRequired; ++i)
    {
        Vector v = frustum.verticies[i];
        VectorStore(v, &boxVerticies[i].pos);
        boxVerticies[i].color = color;
    }

    DebugIndexType* boxIndicies = context->indicies.get() + context->queuedIndicies;
    for (int i = 0; i < indexRequired; i++)
    {
        boxIndicies[i] = gBoxIndexBuffer[i] + static_cast<DebugIndexType>(context->queuedVertices);
    }

    context->queuedVertices += vertexRequired;
    context->queuedIndicies += indexRequired;
}

void DebugRenderer::SetMeshMaterial(DebugRendererContext* context, const Resource::Material* material)
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

    context->commandRecorder->SetPipelineState(mMeshPipelineState.GetPipelineState(macros));

    // FIXME
    // if (tex)
    //    context->commandRecorder->SetTextures(&tex, 1, ShaderType::Pixel);ale n
}

void DebugRenderer::DrawMesh(DebugRendererContext* context, const Resource::Mesh* mesh,
                             const Matrix& matrix)
{
    if (mesh->GetState() != Resource::ResourceState::Loaded)
        return;

    Flush(context);

    IBuffer* vb = mesh->mVB.get();
    IBuffer* ib = mesh->mIB.get();

    if (vb == nullptr || ib == nullptr)
    {
        LOG_ERROR("Invalid vertex or index buffer");
        return;
    }

    if (context->mode != DebugRendererMode::Meshes)
    {
        context->commandRecorder->BindVolatileCBuffer(0, mPerMeshConstantBuffer.get());
        context->mode = DebugRendererMode::Meshes;
        context->polyType = PrimitiveType::Unknown;
    }

    /// update model matrix
    DebugPerMeshCBuffer perMeshCBuffer;
    perMeshCBuffer.modelMatrix = matrix;
    context->commandRecorder->WriteBuffer(mPerMeshConstantBuffer.get(), 0,
                                        sizeof(DebugPerMeshCBuffer),
                                        &perMeshCBuffer);

    int strides[] = { sizeof(Resource::MeshVertex) };
    int offsets[] = { 0 };
    context->commandRecorder->SetVertexBuffers(1, &vb, strides, offsets);
    context->commandRecorder->SetIndexBuffer(ib, IndexBufferFormat::Uint32);
    for (const auto& subMesh : mesh->mSubMeshes)
    {
        SetMeshMaterial(context, subMesh.material);
        context->commandRecorder->DrawIndexed(3 * subMesh.trianglesCount, 1, subMesh.indexOffset);
    }
}

} // namespace Renderer
} // namespace NFE
