/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of high-level Debug Renderer
 */

#pragma once

#include "../PCH.hpp"
#include "DebugRenderer.hpp"
#include "HighLevelRenderer.hpp"
#include "../Globals.hpp"

namespace NFE {
namespace Renderer {

// renderer modules instance definition
std::unique_ptr<DebugRenderer> DebugRenderer::mPtr;


struct NFE_ALIGN16 DebugCBuffer
{
    Matrix viewMatrix;
    Matrix projMatrix;
};


namespace {

size_t gVertexBufferSize = 4096;
size_t gIndexBufferSize = 8192;

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
}

DebugRenderer::DebugRenderer()
{
    IDevice* device = gRenderer->GetDevice();

    BufferDesc bufferDesc;
    ShaderDesc shaderDesc;
    ShaderProgramDesc shaderProgDesc;
    std::string vertexShaderPath = gRenderer->GetShadersPath() + "DebugVS.hlsl";
    std::string pixelShaderPath = gRenderer->GetShadersPath() + "DebugPS.hlsl";

    /// create vertex shader
    shaderDesc.path = vertexShaderPath.c_str();
    shaderDesc.type = ShaderType::Vertex;
    mVertexShader.reset(device->CreateShader(shaderDesc));

    /// create pixel shader
    shaderDesc.path = pixelShaderPath.c_str();
    shaderDesc.type = ShaderType::Pixel;
    mPixelShader.reset(device->CreateShader(shaderDesc));

    /// create shader program
    shaderProgDesc.vertexShader = mVertexShader.get();
    shaderProgDesc.pixelShader = mPixelShader.get();
    mShaderProgram.reset(device->CreateShaderProgram(shaderProgDesc));

    /// create vertex layout
    VertexLayoutElement vertexLayoutElements[] =
    {
        { ElementFormat::Float_32,    3 }, // position
        { ElementFormat::Uint_8_norm, 4 }, // color
        { ElementFormat::Float_32,    2 }, // tex-coords
    };
    VertexLayoutDesc vertexLayoutDesc = { vertexLayoutElements, 3, mVertexShader.get() };
    mVertexLayout.reset(device->CreateVertexLayout(vertexLayoutDesc));


    /// create constant buffer
    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = sizeof(DebugCBuffer);
    bufferDesc.type = BufferType::Constant;
    mConstantBuffer.reset(device->CreateBuffer(bufferDesc));

    /// create vertex buffer
    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = gVertexBufferSize * sizeof(DebugVertex);
    bufferDesc.type = BufferType::Vertex;
    mVertexBuffer.reset(device->CreateBuffer(bufferDesc));

    /// create index buffer
    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = gIndexBufferSize * sizeof(DebugIndexType);
    bufferDesc.type = BufferType::Index;
    mIndexBuffer.reset(device->CreateBuffer(bufferDesc));

    /// create rasterizer state
    RasterizerStateDesc rasterizerStateDesc;
    rasterizerStateDesc.cullMode = CullMode::None;
    rasterizerStateDesc.fillMode = FillMode::Solid;
    mRasterizerState.reset(device->CreateRasterizerState(rasterizerStateDesc));

    // TODO: depth state
}

void DebugRenderer::OnEnter(RenderContext* context)
{
    int stride = sizeof(DebugVertex);
    int offset = 0;
    IBuffer* vb = mVertexBuffer.get();
    context->commandBuffer->SetVertexBuffers(1, &vb, &stride, &offset);
    context->commandBuffer->SetIndexBuffer(mIndexBuffer.get(), IndexBufferFormat::Uint16);
    context->commandBuffer->SetVertexLayout(mVertexLayout.get());
    context->commandBuffer->SetShaderProgram(mShaderProgram.get());

    IBuffer* cb = mConstantBuffer.get();
    context->commandBuffer->SetConstantBuffers(&cb, 1, ShaderType::Vertex);
    context->commandBuffer->SetConstantBuffers(&cb, 1, ShaderType::Pixel);

    context->commandBuffer->SetRasterizerState(mRasterizerState.get());
}

void DebugRenderer::OnLeave(RenderContext* context)
{
    Flush(context);
}

void DebugRenderer::Flush(RenderContext* context)
{
    DebugRendererContext& ctx = context->debugContext;

    if (ctx.queuedVertices == 0 && ctx.queuedIndicies == 0)
        return;

    context->commandBuffer->WriteBuffer(mVertexBuffer.get(), 0,
                                        ctx.queuedVertices * sizeof(DebugVertex),
                                        ctx.vertices.get());

    context->commandBuffer->WriteBuffer(mIndexBuffer.get(), 0,
                                        ctx.queuedIndicies * sizeof(DebugIndexType),
                                        ctx.indicies.get());

    context->commandBuffer->DrawIndexed(PrimitiveType::Lines, ctx.queuedIndicies);

    ctx.queuedVertices = 0;
    ctx.queuedIndicies = 0;
}

void DebugRenderer::SetTarget(RenderContext *context, IRenderTarget* target)
{
    if (target)
    {
        context->commandBuffer->SetRenderTarget(target);
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

} // namespace Renderer
} // namespace NFE
