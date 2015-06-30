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
    mVertices.reset(new DebugVertex[gVertexBufferSize]);
    mIndicies.reset(new DebugIndexType[gIndexBufferSize]);
    mQueuedVertices = 0;
    mQueuedIndicies = 0;
}

DebugRenderer::DebugRenderer()
{
    IDevice* device = gRenderer->GetDevice();

    BufferDesc bufferDesc;
    ShaderDesc shaderDesc;
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

    /// create constant buffer
    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = sizeof(DebugCBuffer);
    bufferDesc.type = BufferType::Constant;
    mVertexBuffer.reset(device->CreateBuffer(bufferDesc));

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
}

void DebugRenderer::OnEnter(RenderContext* context)
{
    // TODO: setting all bufers and rendering states
}

void DebugRenderer::OnLeave(RenderContext* context)
{
    Flush(context);
}

void DebugRenderer::Flush(RenderContext* context)
{
    DebugRendererContext& ctx = context->debugContext;

    if (ctx.mQueuedVertices == 0 && ctx.mQueuedIndicies == 0)
        return;

    context->commandBuffer->WriteBuffer(mVertexBuffer.get(), 0,
                                        ctx.mQueuedVertices * sizeof(DebugVertex),
                                        ctx.mVertices.get());

    context->commandBuffer->WriteBuffer(mIndexBuffer.get(), 0,
                                        ctx.mQueuedIndicies * sizeof(DebugIndexType),
                                        ctx.mIndicies.get());

    // TODO: low-level renderer does not support indexed rendering yet...
    // context->commandBuffer->Draw(PrimitiveType::Lines, ctx.mQueuedVertices, 1, 0, 0, 0);

    ctx.mQueuedVertices = 0;
    ctx.mQueuedIndicies = 0;
}

void DebugRenderer::SetTarget(RenderContext *context, IRenderTarget* target)
{
    // TODO
}

void DebugRenderer::SetCamera(RenderContext *context, const Matrix& viewMatrix,
                              const Matrix& projMatrix)
{
    // TODO
}

void DebugRenderer::DrawLine(RenderContext *context, const Vector& A, const Vector& B,
                             const uint32 color)
{
    // TODO
}

void DebugRenderer::DrawLine(RenderContext *context, const Float3& A, const Float3& B,
                             const uint32 color)
{
    // TODO
}

void DebugRenderer::DrawBox(RenderContext *context, const Box& box, const uint32 color)
{
    const size_t vertexRequired = 8;
    const size_t indexRequired = 24;

    DebugRendererContext& ctx = context->debugContext;

    // check if the box will fit into the buffers
    if ((ctx.mQueuedVertices >= gVertexBufferSize - vertexRequired) ||
        (ctx.mQueuedIndicies >= gIndexBufferSize - indexRequired))
        Flush(context);

    DebugVertex* boxVerticies = ctx.mVertices.get() + ctx.mQueuedVertices;
    for (size_t i = 0; i < vertexRequired; ++i)
    {
        Vector v = box.GetVertex(static_cast<int>(i));
        VectorStore(v, &boxVerticies[i].pos);
        boxVerticies[i].color = color;
    }

    DebugIndexType* boxIndicies = ctx.mIndicies.get() + ctx.mQueuedIndicies;
    for (int i = 0; i < indexRequired; i++)
        boxIndicies[i] = gBoxIndexBuffer[i] + static_cast<DebugIndexType>(ctx.mQueuedVertices);

    ctx.mQueuedVertices += vertexRequired;
    ctx.mQueuedIndicies += indexRequired;
}

void DebugRenderer::DrawFilledBox(RenderContext *context, const Box& box, const uint32 color)
{
    // TODO
}

void DebugRenderer::DrawFrustum(RenderContext *context, const Frustum& frustum, const uint32 color)
{
    // TODO
}

void DebugRenderer::DrawQuad(RenderContext *context, const Vector& pos, ITexture* texture,
                             const uint32 color)
{
    // TODO
}

} // namespace Renderer
} // namespace NFE
