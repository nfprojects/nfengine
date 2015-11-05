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
    vertexBufferSize = 0;
    indexBufferSize = 0;
}

GuiRenderer::GuiRenderer()
{
    IDevice* device = mRenderer->GetDevice();

    mVertexShader.Load("GuiVS");
    mGeometryShader.Load("GuiGS");
    mPixelShader.Load("GuiPS");
    mImGuiVertexShader.Load("ImGuiVS");
    mImGuiPixelShader.Load("ImGuiPS");

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

    VertexLayoutElement imGuiVertexLayoutElements[] =
    {
        { ElementFormat::Float_32,    2, 0,  0, false, 0 }, // pos
        { ElementFormat::Float_32,    2, 8,  0, false, 0 }, // texture coord
        { ElementFormat::Uint_8_norm, 4, 16, 0, false, 0 }, // color
    };
    vertexLayoutDesc.elements = imGuiVertexLayoutElements;
    vertexLayoutDesc.numElements = 3;
    vertexLayoutDesc.vertexShader = mImGuiVertexShader.GetShader(nullptr);
    vertexLayoutDesc.debugName = "GuiRenderer::mImGuiVertexLayout";
    mImGuiVertexLayout.reset(device->CreateVertexLayout(vertexLayoutDesc));

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

    DepthStateDesc dsDesc;
    dsDesc.debugName = "GuiRenderer::mDepthState";
    mDepthState.reset(device->CreateDepthState(dsDesc));

    RasterizerStateDesc rsDesc;
    rsDesc.cullMode = CullMode::Disabled;
    rsDesc.fillMode = FillMode::Solid;
    rsDesc.scissorTest = false;  // TODO
    rsDesc.debugName = "GuiRenderer::mImGuiRasterizerState";
    mImGuiRasterizerState.reset(device->CreateRasterizerState(rsDesc));
}


void GuiRenderer::OnEnter(RenderContext* context)
{
    context->debugContext.mode = DebugRendererMode::Unknown;

    context->commandBuffer->BeginDebugGroup("GUI Renderer stage");

    IBuffer* constantBuffers[] = { mConstantBuffer.get() };
    context->commandBuffer->SetConstantBuffers(constantBuffers, 1, ShaderType::Vertex);
    context->commandBuffer->SetConstantBuffers(constantBuffers, 1, ShaderType::Geometry);

    context->commandBuffer->SetDepthState(mDepthState.get());
    context->commandBuffer->SetBlendState(mBlendState.get());

    ISampler* sampler = mRenderer->GetDefaultSampler();
    context->commandBuffer->SetSamplers(&sampler, 1, ShaderType::Pixel);
}

void GuiRenderer::BeginOrdinaryGuiRendering(RenderContext* context)
{
    IBuffer* vertexBuffers[] = { mVertexBuffer.get() };
    int strides[] = { sizeof(GuiQuadVertex) };
    int offsets[] = { 0 };
    context->commandBuffer->SetVertexBuffers(1, vertexBuffers, strides, offsets);
    context->commandBuffer->SetVertexLayout(mVertexLayout.get());
    context->commandBuffer->SetShader(mVertexShader.GetShader(nullptr));
    context->commandBuffer->SetShader(mGeometryShader.GetShader(nullptr));
    context->commandBuffer->SetRasterizerState(mRenderer->GetDefaultRasterizerState());
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
    context->commandBuffer->SetViewport(0.0f, static_cast<float>(width),
                                        0.0f, static_cast<float>(height),
                                        0.0f, 1.0f);
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

bool GuiRenderer::DrawImGui(RenderContext* context)
{
    GuiRendererContext& ctx = context->guiContext;

    ImGui::Render();
    ImDrawData* drawData = ImGui::GetDrawData();
    if (!drawData)
        return false;

    // grow vertex buffer if too small
    if (!mImGuiVertexBuffer || ctx.vertexBufferSize < drawData->TotalVtxCount)
    {
        const int vertexBufferHysteresis = 5000;
        ctx.vertexBufferSize = drawData->TotalVtxCount + vertexBufferHysteresis;

        BufferDesc bufferDesc;
        bufferDesc.access = BufferAccess::CPU_Write;
        bufferDesc.size = ctx.vertexBufferSize * sizeof(ImDrawVert);
        bufferDesc.type = BufferType::Vertex;
        bufferDesc.debugName = "GuiRenderer::mImGuiVertexBuffer";
        mImGuiVertexBuffer.reset(mRenderer->GetDevice()->CreateBuffer(bufferDesc));
    }

    // grow index buffer if too small
    if (!mImGuiIndexBuffer || ctx.indexBufferSize < drawData->TotalIdxCount)
    {
        const int indexBufferHysteresis = 10000;
        ctx.indexBufferSize = drawData->TotalIdxCount + indexBufferHysteresis;

        BufferDesc bufferDesc;
        bufferDesc.access = BufferAccess::CPU_Write;
        bufferDesc.size = ctx.indexBufferSize * sizeof(ImDrawIdx);
        bufferDesc.type = BufferType::Index;
        bufferDesc.debugName = "GuiRenderer::mImGuiIndexBuffer";
        mImGuiIndexBuffer.reset(mRenderer->GetDevice()->CreateBuffer(bufferDesc));
    }


    // copy vertices and indicies to GPU

    ImDrawVert* vertices = static_cast<ImDrawVert*>(
        context->commandBuffer->MapBuffer(mImGuiVertexBuffer.get(), MapType::WriteOnly));
    ImDrawIdx* indicies = static_cast<ImDrawIdx*>(
        context->commandBuffer->MapBuffer(mImGuiIndexBuffer.get(), MapType::WriteOnly));

    for (int i = 0; i < drawData->CmdListsCount; ++i)
    {
        const ImDrawList* cmdList = drawData->CmdLists[i];
        memcpy(vertices, &cmdList->VtxBuffer[0], cmdList->VtxBuffer.size() * sizeof(ImDrawVert));
        memcpy(indicies, &cmdList->IdxBuffer[0], cmdList->IdxBuffer.size() * sizeof(ImDrawIdx));
        vertices += cmdList->VtxBuffer.size();
        indicies += cmdList->IdxBuffer.size();
    }

    context->commandBuffer->UnmapBuffer(mImGuiVertexBuffer.get());
    context->commandBuffer->UnmapBuffer(mImGuiIndexBuffer.get());


    // setup pipeline

    IBuffer* vertexBuffers[] = { mImGuiVertexBuffer.get() };
    int strides[] = { sizeof(ImDrawVert) };
    int offsets[] = { 0 };
    context->commandBuffer->SetVertexBuffers(1, vertexBuffers, strides, offsets);
    context->commandBuffer->SetIndexBuffer(mImGuiIndexBuffer.get(), IndexBufferFormat::Uint16);
    context->commandBuffer->SetVertexLayout(mImGuiVertexLayout.get());
    context->commandBuffer->SetShader(mImGuiVertexShader.GetShader(nullptr));
    context->commandBuffer->SetShader(mImGuiPixelShader.GetShader(nullptr));
    context->commandBuffer->SetRasterizerState(mImGuiRasterizerState.get());

    context->commandBuffer->SetScissors(0, 600, 800, 0);

    int vertexOffset = 0, indexOffset = 0;
    for (int i = 0; i < drawData->CmdListsCount; ++i)
    {
        const ImDrawList* cmdList = drawData->CmdLists[i];

        for (int j = 0; j < cmdList->CmdBuffer.size(); ++j)
        {
            const ImDrawCmd* command = &cmdList->CmdBuffer[j];
            if (command->UserCallback)
            {
                command->UserCallback(cmdList, command);
            }
            else
            {
                /*
                context->commandBuffer->SetScissors(static_cast<int>(command->ClipRect.x),
                                                    static_cast<int>(command->ClipRect.y),
                                                    static_cast<int>(command->ClipRect.z),
                                                    static_cast<int>(command->ClipRect.w));
                                                    */

                context->commandBuffer->DrawIndexed(PrimitiveType::Triangles,
                                                    command->ElemCount,
                                                    -1,
                                                    indexOffset,
                                                    vertexOffset);
            }
            indexOffset += command->ElemCount;
        }
        vertexOffset += cmdList->VtxBuffer.size();
    }

    return true;
}

} // namespace Renderer
} // namespace NFE
