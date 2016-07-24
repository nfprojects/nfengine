/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of high-level GUI Renderer
 */

#pragma once

#include "PCH.hpp"
#include "GuiRenderer.hpp"
#include "../nfCommon/Logger.hpp"

#include "imgui.h"


namespace NFE {
namespace Renderer {

// renderer modules instance definition
std::unique_ptr<GuiRenderer> GuiRenderer::mPtr;

namespace {

const int VERTEX_BUFFER_HYSTERESIS = 5000;
const int INDEX_BUFFER_HYSTERESIS = 10000;

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

    mShaderProgram.Load("Gui");
    mImGuiShaderProgram.Load("ImGui");

    CreateResourceBindingLayouts();

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
    vertexLayoutDesc.debugName = "GuiRenderer::mImGuiVertexLayout";
    mImGuiVertexLayout.reset(device->CreateVertexLayout(vertexLayoutDesc));

    BufferDesc bufferDesc;
    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = sizeof(GlobalCBuffer);
    bufferDesc.type = BufferType::Constant;
    bufferDesc.debugName = "GuiRenderer::mConstantBuffer";
    mConstantBuffer.reset(device->CreateBuffer(bufferDesc));

    mCBufferBindingInstance.reset(device->CreateResourceBindingInstance(mVSBindingSet.get()));
    if (mCBufferBindingInstance)
        mCBufferBindingInstance->WriteCBufferView(0, mConstantBuffer.get());

    // create dynamic vertex buffer
    bufferDesc.access = BufferAccess::CPU_Write;
    bufferDesc.size = gQuadsBufferSize * sizeof(GuiQuadVertex);
    bufferDesc.type = BufferType::Vertex;
    bufferDesc.debugName = "GuiRenderer::mVertexBuffer";
    mVertexBuffer.reset(device->CreateBuffer(bufferDesc));

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.resBindingLayout = mResBindingLayout.get();
    // blend state that enables additive alpha-blending
    pipelineStateDesc.blendState.rtDescs[0].enable = true;
    pipelineStateDesc.blendState.rtDescs[0].destColorFunc = BlendFunc::OneMinusSrcAlpha;
    pipelineStateDesc.blendState.rtDescs[0].srcColorFunc = BlendFunc::SrcAlpha;
    pipelineStateDesc.raterizerState.cullMode = CullMode::Disabled;
    pipelineStateDesc.raterizerState.fillMode = FillMode::Solid;
    pipelineStateDesc.vertexLayout = mVertexLayout.get();
    pipelineStateDesc.primitiveType = PrimitiveType::Points;
    pipelineStateDesc.debugName = "GuiRenderer::mPipelineState";
    mPipelineState.reset(device->CreatePipelineState(pipelineStateDesc));

    pipelineStateDesc.raterizerState.scissorTest = true;
    pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
    pipelineStateDesc.vertexLayout = mImGuiVertexLayout.get();
    pipelineStateDesc.debugName = "GuiRenderer::mImGuiPipelineState";
    mImGuiPipelineState.reset(device->CreatePipelineState(pipelineStateDesc));
}

bool GuiRenderer::CreateResourceBindingLayouts()
{
    IDevice* device = mRenderer->GetDevice();

    int cbufferSlot = mImGuiShaderProgram.GetResourceSlotByName("VertexCBuffer");
    if (cbufferSlot < 0)
        return false;

    int textureSlot = mImGuiShaderProgram.GetResourceSlotByName("gTexture");
    if (textureSlot < 0)
        return false;

    std::vector<IResourceBindingSet*> bindingSets;

    ResourceBindingDesc vertexShaderBinding(ShaderResourceType::CBuffer, cbufferSlot);
    mVSBindingSet.reset(device->CreateResourceBindingSet(
        ResourceBindingSetDesc(&vertexShaderBinding, 1, ShaderType::Vertex)));
    if (!mVSBindingSet)
        return false;
    bindingSets.push_back(mVSBindingSet.get());

    ResourceBindingDesc pixelShaderBinding(ShaderResourceType::Texture, textureSlot,
                                           mRenderer->GetDefaultSampler());
    mPSBindingSet.reset(device->CreateResourceBindingSet(
        ResourceBindingSetDesc(&pixelShaderBinding, 1, ShaderType::Pixel)));
    if (!mPSBindingSet)
        return false;
    bindingSets.push_back(mPSBindingSet.get());

    // create binding layout
    mResBindingLayout.reset(device->CreateResourceBindingLayout(
        ResourceBindingLayoutDesc(bindingSets.data(), bindingSets.size())));
    if (!mResBindingLayout)
        return false;

    return true;
}

std::unique_ptr<IResourceBindingInstance> GuiRenderer::CreateTextureBinding(ITexture* texture)
{
    IDevice* device = mRenderer->GetDevice();

    std::unique_ptr<IResourceBindingInstance> bindingInstance(
        device->CreateResourceBindingInstance(mPSBindingSet.get()));
    if (!bindingInstance)
        return std::unique_ptr<IResourceBindingInstance>();
    if (!bindingInstance->WriteTextureView(0, texture))
        return std::unique_ptr<IResourceBindingInstance>();

    return bindingInstance;
}

void GuiRenderer::OnEnter(RenderContext* context)
{
    context->debugContext.mode = DebugRendererMode::Unknown;

    context->commandBuffer->BeginDebugGroup("GUI Renderer stage");
}

void GuiRenderer::BeginOrdinaryGuiRendering(RenderContext* context)
{
    IBuffer* vertexBuffers[] = { mVertexBuffer.get() };
    int strides[] = { sizeof(GuiQuadVertex) };
    int offsets[] = { 0 };
    context->commandBuffer->SetVertexBuffers(1, vertexBuffers, strides, offsets);

    context->commandBuffer->SetPipelineState(mPipelineState.get());
}

void GuiRenderer::OnLeave(RenderContext* context)
{
    FlushQueue(context);

    context->commandBuffer->EndDebugGroup();
}

void GuiRenderer::SetTarget(RenderContext* context, IRenderTarget* target)
{
    GlobalCBuffer cbuffer;
    int width, height;
    target->GetDimensions(width, height);
    cbuffer.projMatrix = MatrixOrtho(0.0f, static_cast<float>(width),
                                     static_cast<float>(height), 0.0f,
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
    IResourceBindingInstance* currTextureBinding = nullptr;
    bool currAlphaTexture = false;
    context->commandBuffer->SetShaderProgram(mShaderProgram.GetShaderProgram());

    int firstQuad = 0;
    int packetSize = 0;
    for (size_t i = 0; i < ctx.queuedQuads; ++i)
    {
        if ((ctx.quadData[i].textureBinding != currTextureBinding) ||
            (ctx.quadData[i].alphaTexture != currAlphaTexture))
        {
            // flush quads
            context->commandBuffer->Draw(packetSize, -1, firstQuad);
            packetSize = 0;
            firstQuad = static_cast<int>(i);

            // update texture & shader
            currTextureBinding = ctx.quadData[i].textureBinding;
            currAlphaTexture = ctx.quadData[i].alphaTexture;
            if (currTextureBinding)
            {
                context->commandBuffer->BindResources(1, currTextureBinding);
                macros[0] = currAlphaTexture ? 2 : 1;
            }
            else
                macros[0] = 0;
            context->commandBuffer->SetShaderProgram(mShaderProgram.GetShaderProgram(macros));
        }
        packetSize++;
    }

    if (packetSize > 0)
        context->commandBuffer->Draw(packetSize, -1, firstQuad);

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
                                   const Rectf& texCoords, IResourceBindingInstance* textureBinding,
                                   uint32 color, bool alpha)
{
    PushQuad(context, GuiQuadData(textureBinding, alpha), GuiQuadVertex(rect, texCoords, color));
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
    int offsetY = rect.Ymin;

    int textWidth, textLines;
    font->GetTextSize(text, textWidth, textLines);
    int lineHeight = static_cast<int>(interline * font->mSize);

    (void)hAlign; // TODO

    switch (vAlign)
    {
    case VerticalAlignment::Top:
        offsetY = rect.Ymin;
        break;
    case VerticalAlignment::Center:
        offsetY = rect.Ymin + (rect.Ymax - rect.Ymin - textLines * lineHeight) / 2;
        break;
    case VerticalAlignment::Bottom:
        offsetY = rect.Ymin + (rect.Ymax - rect.Ymin - textLines * lineHeight);
        break;
    default:
        return false;
    }
    offsetY += lineHeight;

    int defaultOffsetX = offsetX;

    GuiQuadData quadData(font->mTextureBinding.get(), true);
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
        quadRect.Ymin = static_cast<float>(offsetY - charInfo.top);
        quadRect.Ymax = static_cast<float>(quadRect.Ymin + charInfo.height);

        Rectf texRect;
        texRect.Xmin = static_cast<float>(charInfo.u) * texInvWidth;
        texRect.Ymin = static_cast<float>(charInfo.v) * texInvHeight;
        texRect.Xmax = static_cast<float>(charInfo.u + charInfo.width) * texInvWidth;
        texRect.Ymax = static_cast<float>(charInfo.v + charInfo.height) * texInvHeight;

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

bool GuiRenderer::DrawImGui(RenderContext* context, IResourceBindingInstance* imGuiTextureBinding)
{
    GuiRendererContext& ctx = context->guiContext;

    ImGui::Render();
    ImDrawData* drawData = ImGui::GetDrawData();
    if (!drawData)
        return false;

    // grow vertex buffer if too small
    if (!mImGuiVertexBuffer || ctx.vertexBufferSize < drawData->TotalVtxCount)
    {
        ctx.vertexBufferSize = drawData->TotalVtxCount + VERTEX_BUFFER_HYSTERESIS;

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
        ctx.indexBufferSize = drawData->TotalIdxCount + INDEX_BUFFER_HYSTERESIS;

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
    context->commandBuffer->SetShaderProgram(mImGuiShaderProgram.GetShaderProgram());
    context->commandBuffer->SetPipelineState(mImGuiPipelineState.get());
    context->commandBuffer->BindResources(0, mCBufferBindingInstance.get());
    context->commandBuffer->BindResources(1, imGuiTextureBinding);

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
                context->commandBuffer->SetScissors(static_cast<int>(command->ClipRect.x),
                                                    static_cast<int>(command->ClipRect.y),
                                                    static_cast<int>(command->ClipRect.z),
                                                    static_cast<int>(command->ClipRect.w));
                context->commandBuffer->DrawIndexed(command->ElemCount,
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