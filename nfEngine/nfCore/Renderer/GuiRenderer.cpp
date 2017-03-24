/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of high-level GUI Renderer
 */

#pragma once

#include "PCH.hpp"
#include "GuiRenderer.hpp"
#include "nfCommon/Logger/Logger.hpp"

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

} // namespace

const size_t GuiRendererContext::gQuadsBufferSize = 1024;


GuiRenderer::GuiRenderer()
{
    IDevice* device = mRenderer->GetDevice();

    mPipelineState.Load("Gui");
    mImGuiPipelineState.Load("ImGui");

    CreateResourceBindingLayouts();

    /// create vertex layout
    VertexLayoutElement vertexLayoutElements[] =
    {
        { ElementFormat::R32G32B32A32_Float,    0,  0, false, 0 }, // rectangle
        { ElementFormat::R32G32B32A32_Float,    16, 0, false, 0 }, // tex-coords
        { ElementFormat::R8G8B8A8_U_Norm,       32, 0, false, 0 }, // color
    };
    VertexLayoutDesc vertexLayoutDesc;
    vertexLayoutDesc.elements = vertexLayoutElements;
    vertexLayoutDesc.numElements = 3;
    vertexLayoutDesc.debugName = "GuiRenderer::mVertexLayout";
    mVertexLayout= device->CreateVertexLayout(vertexLayoutDesc);

    VertexLayoutElement imGuiVertexLayoutElements[] =
    {
        { ElementFormat::R32G32_Float,      0,  0, false, 0 }, // pos
        { ElementFormat::R32G32_Float,      8,  0, false, 0 }, // texture coord
        { ElementFormat::R8G8B8A8_U_Norm,   16, 0, false, 0 }, // color
    };
    vertexLayoutDesc.elements = imGuiVertexLayoutElements;
    vertexLayoutDesc.numElements = 3;
    vertexLayoutDesc.debugName = "GuiRenderer::mImGuiVertexLayout";
    mImGuiVertexLayout= device->CreateVertexLayout(vertexLayoutDesc);

    BufferDesc bufferDesc;
    bufferDesc.mode = BufferMode::Volatile;
    bufferDesc.size = sizeof(GlobalCBuffer);
    bufferDesc.type = BufferType::Constant;
    bufferDesc.debugName = "GuiRenderer::mConstantBuffer";
    mConstantBuffer= device->CreateBuffer(bufferDesc);

    // create dynamic vertex buffer
    bufferDesc.mode = BufferMode::Dynamic;
    bufferDesc.size = GuiRendererContext::gQuadsBufferSize * sizeof(GuiQuadVertex);
    bufferDesc.type = BufferType::Vertex;
    bufferDesc.debugName = "GuiRenderer::mVertexBuffer";
    mVertexBuffer= device->CreateBuffer(bufferDesc);

    PipelineStateDesc pipelineStateDesc;
    pipelineStateDesc.rtFormats[0] = mRenderer->GetBackbufferFormat();
    pipelineStateDesc.resBindingLayout = mResBindingLayout;
    // blend state that enables additive alpha-blending
    pipelineStateDesc.blendState.rtDescs[0].enable = true;
    pipelineStateDesc.blendState.rtDescs[0].destColorFunc = BlendFunc::OneMinusSrcAlpha;
    pipelineStateDesc.blendState.rtDescs[0].srcColorFunc = BlendFunc::SrcAlpha;
    pipelineStateDesc.raterizerState.cullMode = CullMode::Disabled;
    pipelineStateDesc.raterizerState.fillMode = FillMode::Solid;
    pipelineStateDesc.vertexLayout = mVertexLayout;
    pipelineStateDesc.primitiveType = PrimitiveType::Points;
    pipelineStateDesc.debugName = "GuiRenderer::mPipelineState";
    mPipelineState.Build(pipelineStateDesc);

    pipelineStateDesc.raterizerState.scissorTest = true;
    pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
    pipelineStateDesc.vertexLayout = mImGuiVertexLayout;
    pipelineStateDesc.debugName = "GuiRenderer::mImGuiPipelineState";
    mImGuiPipelineState.Build(pipelineStateDesc);
}

bool GuiRenderer::CreateResourceBindingLayouts()
{
    IDevice* device = mRenderer->GetDevice();

    int cbufferSlot = mImGuiPipelineState.GetResourceSlotByName("VertexCBuffer");
    if (cbufferSlot < 0)
        return false;

    int textureSlot = mImGuiPipelineState.GetResourceSlotByName("gTexture");
    if (textureSlot < 0)
        return false;

    std::vector<ResourceBindingSetPtr> bindingSets;

    VolatileCBufferBinding cbufferBindingDesc(ShaderType::All, ShaderResourceType::CBuffer, cbufferSlot, sizeof(GlobalCBuffer));

    ResourceBindingDesc pixelShaderBinding(ShaderResourceType::Texture, textureSlot,
                                           mRenderer->GetDefaultSampler());
    mPSBindingSet = device->CreateResourceBindingSet(
        ResourceBindingSetDesc(&pixelShaderBinding, 1, ShaderType::Pixel));
    if (!mPSBindingSet)
        return false;
    bindingSets.push_back(mPSBindingSet);

    // create binding layout
    mResBindingLayout = device->CreateResourceBindingLayout(
        ResourceBindingLayoutDesc(bindingSets.data(), bindingSets.size(), &cbufferBindingDesc, 1));
    if (!mResBindingLayout)
        return false;

    return true;
}

ResourceBindingInstancePtr GuiRenderer::CreateTextureBinding(const TexturePtr& texture)
{
    IDevice* device = mRenderer->GetDevice();

    ResourceBindingInstancePtr bindingInstance(
        device->CreateResourceBindingInstance(mPSBindingSet));
    if (!bindingInstance)
        return ResourceBindingInstancePtr();
    if (!bindingInstance->WriteTextureView(0, texture))
        return ResourceBindingInstancePtr();

    return bindingInstance;
}

void GuiRenderer::OnEnter(GuiRendererContext* context)
{
    context->commandRecorder->BeginDebugGroup("GUI Renderer stage");
}

void GuiRenderer::BeginOrdinaryGuiRendering(GuiRendererContext* context)
{
    BufferPtr vertexBuffers[] = { mVertexBuffer };
    int strides[] = { sizeof(GuiQuadVertex) };
    int offsets[] = { 0 };
    context->commandRecorder->SetVertexBuffers(1, vertexBuffers, strides, offsets);

    context->commandRecorder->SetPipelineState(mPipelineState.GetPipelineState());
}

void GuiRenderer::OnLeave(GuiRendererContext* context)
{
    FlushQueue(context);

    context->commandRecorder->EndDebugGroup();
}

void GuiRenderer::SetTarget(GuiRendererContext* context, const RenderTargetPtr& target)
{
    GlobalCBuffer cbuffer;
    int width, height;
    target->GetDimensions(width, height);
    cbuffer.projMatrix = Matrix::MakeOrtho(0.0f, static_cast<float>(width),
                                           static_cast<float>(height), 0.0f,
                                           -1.0f, 1.0f);

    // TODO remove
    context->commandRecorder->SetResourceBindingLayout(mResBindingLayout);
    context->commandRecorder->BindVolatileCBuffer(0, mConstantBuffer);
    //

    context->commandRecorder->WriteBuffer(mConstantBuffer, 0, sizeof(GlobalCBuffer),
                                        &cbuffer);

    context->commandRecorder->SetRenderTarget(target);
    context->commandRecorder->SetViewport(0.0f, static_cast<float>(width),
                                        0.0f, static_cast<float>(height),
                                        0.0f, 1.0f);
    context->commandRecorder->SetScissors(0, 0, width, height);
}

void GuiRenderer::FlushQueue(GuiRendererContext* context)
{
    if (context->queuedQuads == 0)
        return;

    // copy quads to vertex buffer
    context->commandRecorder->WriteBuffer(mVertexBuffer, 0,
                                          context->queuedQuads * sizeof(GuiQuadVertex),
                                          context->quadVertices.get());

    int macros[1] = { 0 }; // use texture
    ResourceBindingInstancePtr currTextureBinding = nullptr;
    bool currAlphaTexture = false;
    context->commandRecorder->SetPipelineState(mPipelineState.GetPipelineState());

    int firstQuad = 0;
    int packetSize = 0;
    for (size_t i = 0; i < context->queuedQuads; ++i)
    {
        if ((context->quadData[i].textureBinding != currTextureBinding) ||
            (context->quadData[i].alphaTexture != currAlphaTexture))
        {
            // flush quads
            context->commandRecorder->Draw(packetSize, 1, firstQuad);
            packetSize = 0;
            firstQuad = static_cast<int>(i);

            // update texture & shader
            currTextureBinding = context->quadData[i].textureBinding;
            currAlphaTexture = context->quadData[i].alphaTexture;
            if (currTextureBinding)
            {
                context->commandRecorder->BindResources(0, currTextureBinding);
                macros[0] = currAlphaTexture ? 2 : 1;
            }
            else
                macros[0] = 0;
            context->commandRecorder->SetPipelineState(mPipelineState.GetPipelineState(macros));
        }
        packetSize++;
    }

    if (packetSize > 0)
        context->commandRecorder->Draw(packetSize, 1, firstQuad);

    context->queuedQuads = 0;
}

void GuiRenderer::PushQuad(GuiRendererContext* context, const GuiQuadData& quad,
                           const GuiQuadVertex& quadVertex)
{
    if (context->queuedQuads >= GuiRendererContext::gQuadsBufferSize)
        FlushQueue(context);

    context->quadData[context->queuedQuads] = quad;
    context->quadVertices[context->queuedQuads] = quadVertex;
    context->queuedQuads++;
}

void GuiRenderer::DrawQuad(GuiRendererContext* context, const Rectf& rect, uint32 color)
{
    PushQuad(context, GuiQuadData(), GuiQuadVertex(rect, Rectf(), color));
}

void GuiRenderer::DrawTexturedQuad(GuiRendererContext* context, const Rectf& rect,
                                   const Rectf& texCoords, ResourceBindingInstancePtr textureBinding,
                                   uint32 color, bool alpha)
{
    PushQuad(context, GuiQuadData(textureBinding, alpha), GuiQuadVertex(rect, texCoords, color));
}

bool GuiRenderer::PrintText(GuiRendererContext* context, Font* font, const char* text,
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

    GuiQuadData quadData(font->mTextureBinding, true);
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

bool GuiRenderer::PrintTextWithBorder(GuiRendererContext* context, Font* font, const char* text,
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

bool GuiRenderer::DrawImGui(GuiRendererContext* context, ResourceBindingInstancePtr imGuiTextureBinding)
{
    ImGui::Render();
    ImDrawData* drawData = ImGui::GetDrawData();
    if (!drawData)
        return false;

    // grow vertex buffer if too small
    if (!mImGuiVertexBuffer || context->vertexBufferSize < drawData->TotalVtxCount)
    {
        context->vertexBufferSize = drawData->TotalVtxCount + VERTEX_BUFFER_HYSTERESIS;

        BufferDesc bufferDesc;
        bufferDesc.mode = BufferMode::Dynamic;
        bufferDesc.size = context->vertexBufferSize * sizeof(ImDrawVert);
        bufferDesc.type = BufferType::Vertex;
        bufferDesc.debugName = "GuiRenderer::mImGuiVertexBuffer";
        mImGuiVertexBuffer= mRenderer->GetDevice()->CreateBuffer(bufferDesc);
    }

    // grow index buffer if too small
    if (!mImGuiIndexBuffer || context->indexBufferSize < drawData->TotalIdxCount)
    {
        context->indexBufferSize = drawData->TotalIdxCount + INDEX_BUFFER_HYSTERESIS;

        BufferDesc bufferDesc;
        bufferDesc.mode = BufferMode::Dynamic;
        bufferDesc.size = context->indexBufferSize * sizeof(ImDrawIdx);
        bufferDesc.type = BufferType::Index;
        bufferDesc.debugName = "GuiRenderer::mImGuiIndexBuffer";
        mImGuiIndexBuffer= mRenderer->GetDevice()->CreateBuffer(bufferDesc);
    }

    // copy vertices and indicies to GPU
    size_t vbOffset = 0;
    size_t ibOffset = 0;
    for (int i = 0; i < drawData->CmdListsCount; ++i)
    {
        const ImDrawList* cmdList = drawData->CmdLists[i];

        context->commandRecorder->WriteBuffer(mImGuiVertexBuffer,
                                            vbOffset * sizeof(ImDrawVert),
                                            cmdList->VtxBuffer.size() * sizeof(ImDrawVert),
                                            &cmdList->VtxBuffer[0]);

        context->commandRecorder->WriteBuffer(mImGuiIndexBuffer,
                                            ibOffset * sizeof(ImDrawIdx),
                                            cmdList->IdxBuffer.size() * sizeof(ImDrawIdx),
                                            &cmdList->IdxBuffer[0]);

        vbOffset += cmdList->VtxBuffer.size();
        ibOffset += cmdList->IdxBuffer.size();
    }

    // setup pipeline

    BufferPtr vertexBuffers[] = { mImGuiVertexBuffer };
    int strides[] = { sizeof(ImDrawVert) };
    int offsets[] = { 0 };
    context->commandRecorder->SetVertexBuffers(1, vertexBuffers, strides, offsets);
    context->commandRecorder->SetIndexBuffer(mImGuiIndexBuffer, IndexBufferFormat::Uint16);
    context->commandRecorder->SetResourceBindingLayout(mResBindingLayout);
    context->commandRecorder->SetPipelineState(mImGuiPipelineState.GetPipelineState());
    context->commandRecorder->BindVolatileCBuffer(0, mConstantBuffer);
    context->commandRecorder->BindResources(0, imGuiTextureBinding);

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
                context->commandRecorder->SetScissors(static_cast<int>(command->ClipRect.x),
                                                    static_cast<int>(command->ClipRect.y),
                                                    static_cast<int>(command->ClipRect.z),
                                                    static_cast<int>(command->ClipRect.w));
                context->commandRecorder->DrawIndexed(command->ElemCount,
                                                    1,
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
