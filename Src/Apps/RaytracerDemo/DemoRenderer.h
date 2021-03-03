#pragma once

#include "Engine/Renderers/RendererCommon/Device.hpp"
#include "Engine/Common/System/Library.hpp"

namespace NFE {

class DemoRenderer
{
public:

    struct DrawParams
    {
        const void* imageData;
        uint32 imageWidth;
        uint32 imageHeight;
        uint32 imageStride;
    };

    DemoRenderer();

    bool Init(const Common::Window& window);
    void SetupFontTexture(uint32 width, uint32 height, const void* data);

    void OnResize(const Common::Window& window);
    void Release();

    void Draw(const DrawParams& params);

private:

    void CreateResources(const Common::Window& window);

    NFE::Common::Library mRendererLib;
    Renderer::IDevice* mDevice = nullptr;

    Renderer::CommandQueuePtr mQueue;
    Renderer::CommandRecorderPtr mCommandRecorder;

    Renderer::BackbufferPtr mBackbuffer;

    Renderer::TexturePtr mTexture;
    Renderer::RenderTargetPtr mRenderTarget;

    // ImGui rendering related

    Renderer::VertexLayoutPtr mVertexLayout;
    Renderer::ShaderPtr mVertexShader;
    Renderer::ShaderPtr mPixelShader;
    Renderer::SamplerPtr mSampler;
    Renderer::ResourceBindingLayoutPtr mBindingLayout;
    Renderer::PipelineStatePtr mPipelineState;
    Renderer::BufferPtr mCBuffer;
    Renderer::BufferPtr mVertexBuffer;
    Renderer::BufferPtr mIndexBuffer;
    Renderer::TexturePtr mFontAtlasTexture;
};

} // namespace NFE