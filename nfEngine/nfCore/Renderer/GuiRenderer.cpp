/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of high-level GUI Renderer
 */

#pragma once

#include "../PCH.hpp"
#include "GuiRenderer.hpp"

namespace NFE {
namespace Renderer {

// renderer modules instance definition
std::unique_ptr<GuiRenderer> GuiRenderer::mPtr;

namespace {

struct GlobalCBuffer
{
    Matrix projMatrix;
};

size_t gQuadsBufferSize = 1024;

} // namespace

GuiRendererContext::GuiRendererContext()
{
    quadData.reset(new GuiQuadData[gQuadsBufferSize]);
    quadVertices.reset(new GuiQuadVertex[gQuadsBufferSize]);
    queuedQuads = 0;
}

GuiRenderer::GuiRenderer()
{
    IDevice* device = mRenderer->GetDevice();

    mVertexShader.Load("GuiVS");
    mGeometryShader.Load("GuiGS");
    mPixelShader.Load("GuiPS");

    /// create vertex layout
    VertexLayoutElement vertexLayoutElements[] =
    {
        { ElementFormat::Float_32,    4, 0,  0, false, 0 }, // rectangle
        { ElementFormat::Float_32,    4, 16, 0, false, 0 }, // tex-coords
        { ElementFormat::Uint_8_norm, 4, 32, 0, false, 0 }, // color
    };
    VertexLayoutDesc vertexLayoutDesc;
    vertexLayoutDesc.elements = vertexLayoutElements;
    vertexLayoutDesc.numElements = 3;
    vertexLayoutDesc.vertexShader = mVertexShader.GetShader(nullptr);
    vertexLayoutDesc.debugName = "GuiRenderer::mVertexLayout";
    mVertexLayout.reset(device->CreateVertexLayout(vertexLayoutDesc));

    BufferDesc bufferDesc;
    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = sizeof(GlobalCBuffer);
    bufferDesc.type = BufferType::Constant;
    bufferDesc.debugName = "GuiRenderer::mConstantBuffer";
    mConstantBuffer.reset(device->CreateBuffer(bufferDesc));

    // create dynamic vertex buffer
    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = gQuadsBufferSize * sizeof(GuiQuadVertex);
    bufferDesc.type = BufferType::Vertex;
    bufferDesc.debugName = "GuiRenderer::mVertexBuffer";
    mVertexBuffer.reset(device->CreateBuffer(bufferDesc));

    // blend state that enables additive alpha-blending
    BlendStateDesc bsDesc;
    bsDesc.rtDescs[0].enable = true;
    bsDesc.rtDescs[0].destColorFunc = BlendFunc::OneMinusSrcAlpha;
    bsDesc.rtDescs[0].srcColorFunc = BlendFunc::SrcAlpha;
    bsDesc.debugName = "GuiRenderer::mBlendState";
    mBlendState.reset(device->CreateBlendState(bsDesc));
}


void GuiRenderer::OnEnter(RenderContext* context)
{
    context->debugContext.mode = DebugRendererMode::Unknown;

    context->commandBuffer->BeginDebugGroup("GUI Renderer stage");

    context->commandBuffer->SetShader(mVertexShader.GetShader(nullptr));
    context->commandBuffer->SetShader(mGeometryShader.GetShader(nullptr));

    IBuffer* constantBuffers[] = { mConstantBuffer.get() };
    context->commandBuffer->SetConstantBuffers(constantBuffers, 1, ShaderType::Geometry);

    context->commandBuffer->SetDepthState(mRenderer->GetDefaultDepthState());
    context->commandBuffer->SetBlendState(mBlendState.get());

    ISampler* sampler = mRenderer->GetDefaultSampler();
    context->commandBuffer->SetSamplers(&sampler, 1, ShaderType::Pixel);

    IBuffer* veretexBuffers[] = { mVertexBuffer.get() };
    int strides[] = { sizeof(GuiQuadVertex) };
    int offsets[] = { 0 };
    context->commandBuffer->SetVertexBuffers(1, veretexBuffers, strides, offsets);
    context->commandBuffer->SetVertexLayout(mVertexLayout.get());
}

void GuiRenderer::OnLeave(RenderContext* context)
{
    FlushQueue(context);

    // TODO: we only need to reset geometry shader
    context->commandBuffer->Reset();

    context->commandBuffer->EndDebugGroup();
}

void GuiRenderer::SetTarget(RenderContext* context, IRenderTarget* target)
{
    GlobalCBuffer cbuffer;
    int width, height;
    target->GetDimensions(width, height);
    cbuffer.projMatrix = MatrixOrtho(0.0f, static_cast<float>(width),
                                     0.0f, static_cast<float>(height),
                                     -1.0f, 1.0f);
    context->commandBuffer->WriteBuffer(mConstantBuffer.get(), 0, sizeof(GlobalCBuffer),
                                        &cbuffer);

    context->commandBuffer->SetRenderTarget(target);
}

void GuiRenderer::FlushQueue(RenderContext* context)
{
    GuiRendererContext& ctx = context->guiContext;

    if (ctx.queuedQuads == 0)
        return;

    // copy quads to vertex buffer
    context->commandBuffer->WriteBuffer(mVertexBuffer.get(), 0,
                                        ctx.queuedQuads * sizeof(GuiQuadVertex),
                                        ctx.quadVertices.get());

    int macros[1] = { 0 }; // use texture
    ITexture* currTexture = nullptr;
    bool currAlphaTexture = false;
    context->commandBuffer->SetShader(mPixelShader.GetShader(macros));

    int firstQuad = 0;
    int packetSize = 0;
    for (size_t i = 0; i < ctx.queuedQuads; ++i)
    {
        if ((ctx.quadData[i].texture != currTexture) ||
            (ctx.quadData[i].alphaTexture != currAlphaTexture))
        {
            // flush quads
            context->commandBuffer->Draw(PrimitiveType::Points, packetSize, -1, firstQuad);
            packetSize = 0;
            firstQuad = static_cast<int>(i);

            // update texture & shader
            currTexture = ctx.quadData[i].texture;
            currAlphaTexture = ctx.quadData[i].alphaTexture;
            if (currTexture)
            {
                context->commandBuffer->SetTextures(&currTexture, 1, ShaderType::Pixel);
                macros[0] = currAlphaTexture ? 2 : 1;
            }
            else
                macros[0] = 0;
            context->commandBuffer->SetShader(mPixelShader.GetShader(macros));
        }
        packetSize++;
    }

    if (packetSize > 0)
        context->commandBuffer->Draw(PrimitiveType::Points, packetSize, -1, firstQuad);

    ctx.queuedQuads = 0;
}

void GuiRenderer::PushQuad(RenderContext* context, const GuiQuadData& quad,
                           const GuiQuadVertex& quadVertex)
{
    GuiRendererContext& ctx = context->guiContext;

    if (ctx.queuedQuads >= gQuadsBufferSize)
        FlushQueue(context);

    ctx.quadData[ctx.queuedQuads] = quad;
    ctx.quadVertices[ctx.queuedQuads] = quadVertex;
    ctx.queuedQuads++;
}

void GuiRenderer::DrawQuad(RenderContext* context, const Rectf& rect, uint32 color)
{
    PushQuad(context, GuiQuadData(), GuiQuadVertex(rect, Rectf(), color));
}

void GuiRenderer::DrawTexturedQuad(RenderContext* context, const Rectf& rect,
                                   const Rectf& texCoords, ITexture* texture,
                                   uint32 color, bool alpha)
{
    PushQuad(context, GuiQuadData(texture, alpha), GuiQuadVertex(rect, texCoords, color));
}

bool GuiRenderer::PrintText(RenderContext* context, Font* font, const char* text,
                            const Recti& rect, uint32 color,
                            VerticalAlignment vAlign, HorizontalAlignment hAlign)
{
    if (!font || !font->mTexture)
        return false;

    const float interline = 1.5f;
    float texInvWidth = 1.0f / (float)font->mTexWidth;
    float texInvHeight = 1.0f / (float)font->mTexHeight;

    int offsetX = rect.Xmin;
    int offsetY = rect.Ymax;

    int textWidth, textLines;
    font->GetTextSize(text, textWidth, textLines);
    int lineHeight = static_cast<int>(interline * font->mSize);

    (void)hAlign; // TODO

    switch (vAlign)
    {
    case VerticalAlignment::Top:
        offsetY = rect.Ymax;
        break;
    case VerticalAlignment::Center:
        offsetY = rect.Ymax - (rect.Ymax - rect.Ymin - textLines * lineHeight) / 2;
        break;
    case VerticalAlignment::Bottom:
        offsetY = rect.Ymax - (rect.Ymax - rect.Ymin - textLines * lineHeight);
        break;
    default:
        return false;
    }
    offsetY -= lineHeight;

    int defaultOffsetX = offsetX;

    GuiQuadData quadData(font->mTexture.get(), true);
    for (int i = 0; text[i]; i++)
    {
        // TODO: UTF-8 support
        size_t charId = static_cast<unsigned char>(text[i]);
        if (charId > 127)
            charId = '?';

        if (charId == '\n')  // carriage return
        {
            offsetY -= lineHeight;
            offsetX = defaultOffsetX;
            continue;
        }

        const CharacterInfo& charInfo = font->mChars[charId];
        if (charInfo.width < 0)
            continue;

        Rectf quadRect;
        quadRect.Xmin = static_cast<float>(offsetX + charInfo.left);
        quadRect.Xmax = static_cast<float>(quadRect.Xmin + charInfo.width);
        quadRect.Ymin = static_cast<float>(offsetY + charInfo.top - charInfo.height);
        quadRect.Ymax = static_cast<float>(quadRect.Ymin + charInfo.height);

        Rectf texRect;
        texRect.Xmin = static_cast<float>(charInfo.u) * texInvWidth;
        texRect.Ymax = static_cast<float>(charInfo.v) * texInvHeight;
        texRect.Xmax = static_cast<float>(charInfo.u + charInfo.width) * texInvWidth;
        texRect.Ymin = static_cast<float>(charInfo.v + charInfo.height) * texInvHeight;

        PushQuad(context, quadData, GuiQuadVertex(quadRect, texRect, color));
        offsetX += charInfo.spacing;
    }

    return true;
}

bool GuiRenderer::PrintTextWithBorder(RenderContext* context, Font* font, const char* text,
                                      const Recti& rect, uint32 color, uint32 borderColor,
                                      VerticalAlignment vAlign, HorizontalAlignment hAlign)
{
    Recti tempRect;
    for (int i = -1; i <= 1; ++i)
    {
        for (int j = -1; j <= 1; ++j)
        {
            if (i == 0 && j == 0)
                continue;

            tempRect.Ymin = rect.Ymin + j;
            tempRect.Ymax = rect.Ymax + j;
            tempRect.Xmin = rect.Xmin + i;
            tempRect.Xmax = rect.Xmax + i;
            PrintText(context, font, text, tempRect, borderColor, vAlign, hAlign);
        }
    }

    return PrintText(context, font, text, rect, color, vAlign, hAlign);
}

} // namespace Renderer
} // namespace NFE
