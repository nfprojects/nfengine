#include "PCH.h"
#include "DemoRenderer.h"
#include "Engine/Common/System/Window.hpp"

#include <imgui/imgui.h>

namespace NFE {

using namespace Renderer;

static const char* c_vertexShaderCode = R"(
cbuffer vertexBuffer : register(b0)
{
    float4x4 cProjectionMatrix;
};
struct VS_INPUT
{
    float2 pos : POSITION;
    float2 uv  : TEXCOORD0;
    float4 col : TEXCOORD1;
};
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = mul(cProjectionMatrix, float4(input.pos.xy, 0.0f, 1.0f));
    output.col = input.col;
    output.uv  = input.uv;
    return output;
})";

static const char* c_pixelShaderCode = R"(
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};
SamplerState sampler0 : register(s0);
Texture2D texture0 : register(t0);
float4 main(PS_INPUT input) : SV_Target
{
    float4 out_col = input.col * texture0.Sample(sampler0, input.uv);
    return out_col;
})";

static const Format c_renderTargetFormat = Format::R8G8B8A8_U_Norm_sRGB;

DemoRenderer::DemoRenderer()
{
}

bool DemoRenderer::Init(const Common::Window& window)
{
    if (!mRendererLib.Open("RendererD3D12"))
    {
        return false;
    }

    RendererInitFunc rendererInitFunc;
    if (!mRendererLib.GetSymbol(RENDERER_INIT_FUNC, rendererInitFunc))
    {
        return false;
    }

    DeviceInitParams params;
    params.debugLevel = 2;
    mDevice = rendererInitFunc(&params);
    if (!mDevice)
    {
        return false;
    }

    mQueue = mDevice->CreateCommandQueue(CommandQueueType::Graphics, "Queue");
    NFE_ASSERT(mQueue, "Failed to create command queue");

    mCommandRecorder = mDevice->CreateCommandRecorder();
    NFE_ASSERT(mCommandRecorder, "Failed to create command recorder");

    {
        BackbufferDesc desc;
        desc.windowHandle = window.GetHandle();
        desc.commandQueue = mQueue;
        desc.width = window.GetWidth();
        desc.height = window.GetHeight();
        desc.debugName = "DemoRenderer::mBackbuffer";
        desc.format = Format::R8G8B8A8_U_Norm;
        desc.vSyncInterval = 1u;

        mBackbuffer = mDevice->CreateBackbuffer(desc);
        NFE_ASSERT(mBackbuffer, "Failed to create backbuffer");
    }

    CreateResources(window);

    // vertex layout
    {
        VertexLayoutElement vertexLayoutElements[] =
        {
            { Format::R32G32_Float,     0, 0, false, 0 },   // position
            { Format::R32G32_Float,     8, 0, false, 0 },   // tex-coords
            { Format::R8G8B8A8_U_Norm,  16, 0, false, 0 },  // color
        };

        VertexLayoutDesc vertexLayoutDesc;
        vertexLayoutDesc.elements = vertexLayoutElements;
        vertexLayoutDesc.numElements = 3;
        mVertexLayout = mDevice->CreateVertexLayout(vertexLayoutDesc);
        NFE_ASSERT(mVertexLayout, "Failed to create vertex layout");
    }

    // vertex shader
    {
        ShaderDesc desc;
        desc.type = ShaderType::Vertex;
        desc.code = c_vertexShaderCode;
        desc.path = "vertexShader";
        mVertexShader = mDevice->CreateShader(desc);
        NFE_ASSERT(mVertexShader, "Failed to create VS");
    }

    // pixel shader
    {
        ShaderDesc desc;
        desc.type = ShaderType::Pixel;
        desc.code = c_pixelShaderCode;
        desc.path = "pixelShader";
        mPixelShader = mDevice->CreateShader(desc);
        NFE_ASSERT(mPixelShader, "Failed to create PS");
    }

    // sampler
    {
        SamplerDesc samplerDesc;
        mSampler = mDevice->CreateSampler(samplerDesc);
        NFE_ASSERT(mSampler, "Failed to create sampler");
    }

    // binding layout
    {
        const VolatileCBufferBinding volatileCBufferBinding(ShaderType::Vertex, ShaderResourceType::CBuffer, 0, 16 * 4);

        ResourceBindingDesc pixelShaderBindingDesc(ShaderResourceType::Texture, 0, mSampler);
        const ResourceBindingSetPtr pixelShaderBinding = mDevice->CreateResourceBindingSet(ResourceBindingSetDesc(&pixelShaderBindingDesc, 1, ShaderType::Pixel));
        NFE_ASSERT(pixelShaderBinding, "Failed to create PS binding set");

        mBindingLayout = mDevice->CreateResourceBindingLayout(ResourceBindingLayoutDesc(&pixelShaderBinding, 1u, &volatileCBufferBinding, 1u));
        NFE_ASSERT(mBindingLayout, "Failed to create binding layout");
    }

    // pipeline state object
    {
        PipelineStateDesc pipelineStateDesc;
        pipelineStateDesc.renderTargetFormats = { c_renderTargetFormat };
        pipelineStateDesc.vertexShader = mVertexShader;
        pipelineStateDesc.pixelShader = mPixelShader;
        pipelineStateDesc.blendState.independent = false;
        pipelineStateDesc.blendState.rtDescs[0].enable = true;
        pipelineStateDesc.blendState.rtDescs[0].srcColorFunc = BlendFunc::SrcAlpha;
        pipelineStateDesc.blendState.rtDescs[0].destColorFunc = BlendFunc::OneMinusSrcAlpha;
        pipelineStateDesc.blendState.rtDescs[0].destAlphaFunc = BlendFunc::OneMinusSrcAlpha;
        pipelineStateDesc.primitiveType = PrimitiveType::Triangles;
        pipelineStateDesc.vertexLayout = mVertexLayout;
        pipelineStateDesc.resBindingLayout = mBindingLayout;

        mPipelineState = mDevice->CreatePipelineState(pipelineStateDesc);
        NFE_ASSERT(mPipelineState, "Failed to create pipeline state");
    }

    // constant buffer
    {
        BufferDesc desc;
        desc.debugName = "CBuffer";
        desc.mode = ResourceAccessMode::Volatile;
        desc.size = 16 * 4;
        desc.usage = BufferUsageFlag::ConstantBuffer;

        mCBuffer = mDevice->CreateBuffer(desc);
        NFE_ASSERT(mCBuffer, "Failed to create cbuffer");
    }

    // vertex buffer
    {
        BufferDesc desc;
        desc.debugName = "VertexBuffer";
        desc.mode = ResourceAccessMode::GPUOnly; // TODO volatile
        desc.size = 64 * 1024 * sizeof(ImDrawVert);
        desc.usage = BufferUsageFlag::VertexBuffer;
        desc.structSize = sizeof(ImDrawVert);

        mVertexBuffer = mDevice->CreateBuffer(desc);
        NFE_ASSERT(mVertexBuffer, "Failed to create vertex buffer");
    }

    // index buffer
    {
        BufferDesc desc;
        desc.debugName = "IndexBuffer";
        desc.mode = ResourceAccessMode::GPUOnly; // TODO volatile
        desc.size = 64 * 1024 * sizeof(ImDrawIdx);
        desc.usage = BufferUsageFlag::IndexBuffer;

        mIndexBuffer = mDevice->CreateBuffer(desc);
        NFE_ASSERT(mIndexBuffer, "Failed to create index buffer");
    }

    return true;
}

void DemoRenderer::SetupFontTexture(uint32 width, uint32 height, const void* data)
{
    TextureDesc desc;
    desc.mode = ResourceAccessMode::Immutable;
    desc.width = width;
    desc.height = height;
    desc.format = Format::R8G8B8A8_U_Norm;
    desc.debugName = "FontAtlas";

    mFontAtlasTexture = mDevice->CreateTexture(desc);
    NFE_ASSERT(mFontAtlasTexture, "Failed to create texture");

    mCommandRecorder->Begin(CommandQueueType::Graphics);
    mCommandRecorder->WriteTexture(mFontAtlasTexture, data);
    const CommandListPtr cl = mCommandRecorder->Finish();

    mQueue->Execute(cl);
    mQueue->Signal()->Wait();
}

void DemoRenderer::Release()
{
    mQueue->Signal()->Wait();

    mBackbuffer.Reset();
    mQueue.Reset();
    mCommandRecorder.Reset();
    mBackbuffer.Reset();
    mTexture.Reset();
    mRenderTarget.Reset();
    mVertexLayout.Reset();
    mVertexShader.Reset();
    mPixelShader.Reset();
    mSampler.Reset();
    mBindingLayout.Reset();
    mPipelineState.Reset();
    mCBuffer.Reset();
    mVertexBuffer.Reset();
    mIndexBuffer.Reset();
    mFontAtlasTexture.Reset();

    // free Renderer
    if (mDevice)
    {
        RendererReleaseFunc proc;
        if (mRendererLib.GetSymbol(RENDERER_RELEASE_FUNC, proc))
        {
            proc();
        }

        mDevice = nullptr;
    }
}

void DemoRenderer::CreateResources(const Common::Window& window)
{
    if (!mDevice)
    {
        return;
    }

    {
        TextureDesc desc;
        desc.width = window.GetWidth();
        desc.height = window.GetHeight();
        desc.debugName = "DemoRenderer::mTexture";
        desc.format = c_renderTargetFormat;
        desc.usage = TextureUsageFlag::RenderTarget;
        desc.mode = ResourceAccessMode::GPUOnly;

        mTexture = mDevice->CreateTexture(desc);
        NFE_ASSERT(mTexture, "Failed to create texture");
    }

    {
        RenderTargetDesc desc;
        desc.targets = { RenderTargetElement(mTexture) };
        mRenderTarget = mDevice->CreateRenderTarget(desc);
        NFE_ASSERT(mRenderTarget, "Failed to create render target");
    }
}

void DemoRenderer::OnResize(const Common::Window& window)
{
    if (mBackbuffer)
    {
        mBackbuffer->Resize(window.GetWidth(), window.GetHeight());
    }

    CreateResources(window);
}

void DemoRenderer::Draw(const DrawParams& params)
{
    mCommandRecorder->Begin(CommandQueueType::Graphics);

    TextureRegion region;
    region.width = params.imageWidth;
    region.height = params.imageHeight;

    mCommandRecorder->WriteTexture(mTexture, params.imageData, &region, params.imageStride);

    mCommandRecorder->SetRenderTarget(mRenderTarget);
    mCommandRecorder->SetPipelineState(mPipelineState);
    mCommandRecorder->SetResourceBindingLayout(PipelineType::Graphics, mBindingLayout);

    {
        const uint32 stride = sizeof(ImDrawVert);
        const uint32 offset = 0u;
        mCommandRecorder->SetVertexBuffers(1u, &mVertexBuffer, &stride, &offset);
        mCommandRecorder->SetIndexBuffer(mIndexBuffer, IndexBufferFormat::Uint16);
    }

    mCommandRecorder->BindTexture(PipelineType::Graphics, 0, 0, mFontAtlasTexture);

    // draw ImGui
    if (const ImDrawData* draw_data = ImGui::GetDrawData())
    {
        mCommandRecorder->SetViewport(0.0f, draw_data->DisplaySize.x, 0.0f, draw_data->DisplaySize.y, 0.0f, 1.0f);

        {
            float L = draw_data->DisplayPos.x;
            float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
            float T = draw_data->DisplayPos.y;
            float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;

            const float cbufferData[4][4] =
            {
                { 2.0f / (R - L),       0.0f,               0.0f,       0.0f },
                { 0.0f,                 2.0f / (T - B),     0.0f,       0.0f },
                { 0.0f,                 0.0f,               0.5f,       0.0f },
                { (R + L) / (L - R),    (T + B) / (B - T),  0.5f,       1.0f },
            };

            mCommandRecorder->BindVolatileCBuffer(PipelineType::Graphics, 0, mCBuffer);
            mCommandRecorder->WriteBuffer(mCBuffer, 0, 16 * 4, cbufferData);
        }

        // Upload vertex/index data into a single contiguous GPU buffer
        uint32 vertexOffset = 0;
        uint32 indexOffset = 0;
        for (int32 n = 0; n < draw_data->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];

            mCommandRecorder->WriteBuffer(
                mVertexBuffer,
                vertexOffset * sizeof(ImDrawVert),
                cmd_list->VtxBuffer.Size * sizeof(ImDrawVert),
                cmd_list->VtxBuffer.Data);

            mCommandRecorder->WriteBuffer(
                mIndexBuffer,
                indexOffset * sizeof(ImDrawIdx),
                cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx),
                cmd_list->IdxBuffer.Data);

            vertexOffset += cmd_list->VtxBuffer.Size;
            indexOffset += cmd_list->IdxBuffer.Size;
        }

        // Render command lists
        // (Because we merged all buffers into a single one, we maintain our own offset into them)
        int32 global_vtx_offset = 0;
        int32 global_idx_offset = 0;
        ImVec2 clipOffset = draw_data->DisplayPos;
        for (int32 n = 0; n < draw_data->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];
            for (int32 cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
            {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback != NULL)
                {
                    // User callback, registered via ImDrawList::AddCallback()
                    // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                    if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    {
                        // TODO
                    }
                    else
                    {
                        pcmd->UserCallback(cmd_list, pcmd);
                    }
                }
                else
                {
                    const int32 left =      (int32)(pcmd->ClipRect.x - clipOffset.x);
                    const int32 top =       (int32)(pcmd->ClipRect.y - clipOffset.y);
                    const int32 right =     (int32)(pcmd->ClipRect.z - clipOffset.x);
                    const int32 bottom =    (int32)(pcmd->ClipRect.w - clipOffset.y);

                    if (right > left && bottom > top)
                    {
                        //ctx->SetGraphicsRootDescriptorTable(1, *(D3D12_GPU_DESCRIPTOR_HANDLE*)&pcmd->TextureId);
                        mCommandRecorder->SetScissors(left, top, right, bottom);
                        mCommandRecorder->DrawIndexed(pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0u);
                    }
                }
            }
            global_idx_offset += cmd_list->IdxBuffer.Size;
            global_vtx_offset += cmd_list->VtxBuffer.Size;
        }
    }

    mCommandRecorder->CopyTexture(mTexture, mBackbuffer);

    CommandListPtr cl = mCommandRecorder->Finish();
    NFE_ASSERT(cl, "Failed to record command list");

    mQueue->Execute(cl);

    if (mBackbuffer)
    {
        mBackbuffer->Present();
    }
}

} // namespace NFE
