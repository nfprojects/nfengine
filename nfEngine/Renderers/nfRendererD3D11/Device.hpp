/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 11 render's device.
 */

#pragma once

#include "../RendererInterface/Device.hpp"
#include "Common.hpp"


namespace NFE {
namespace Renderer {

class Device : public IDevice
{
    friend class Backbuffer;
    friend class RenderTarget;

    D3DPtr<ID3D11Device> mDevice;
    D3DPtr<ID3D11DeviceContext> mImmediateContext;
    D3DPtr<IDXGIFactory> mDXGIFactory;
    D3DPtr<ID3D11InfoQueue> mInfoQueue;

    std::vector<D3DPtr<IDXGIAdapter>> mAdapters;
    int mAdapterInUse;

    D3D_FEATURE_LEVEL mFeatureLevel;
    bool mDebugLayerEnabled;

    bool DetectVideoCards(int preferredId);

public:
    Device();
    ~Device();

    bool Init(const DeviceInitParams* params);

    void* GetHandle() const override;
    ID3D11Device* Get() const;
    bool GetDeviceInfo(DeviceInfo& info) override;
    bool IsBackbufferFormatSupported(ElementFormat format) override;

    /// Resources creation functions

    VertexLayoutPtr CreateVertexLayout(const VertexLayoutDesc& desc) override;
    BufferPtr CreateBuffer(const BufferDesc& desc) override;
    TexturePtr CreateTexture(const TextureDesc& desc) override;
    BackbufferPtr CreateBackbuffer(const BackbufferDesc& desc) override;
    RenderTargetPtr CreateRenderTarget(const RenderTargetDesc& desc) override;
    PipelineStatePtr CreatePipelineState(const PipelineStateDesc& desc) override;
    ComputePipelineStatePtr CreateComputePipelineState(const ComputePipelineStateDesc& desc) override;
    SamplerPtr CreateSampler(const SamplerDesc& desc) override;
    ShaderPtr CreateShader(const ShaderDesc& desc) override;
    ResourceBindingSetPtr CreateResourceBindingSet(const ResourceBindingSetDesc& desc) override;
    ResourceBindingLayoutPtr CreateResourceBindingLayout(const ResourceBindingLayoutDesc& desc) override;
    ResourceBindingInstancePtr CreateResourceBindingInstance(ResourceBindingSetPtr set) override;

    CommandRecorderPtr CreateCommandRecorder() override;
    bool Execute(CommandListID commandList) override;
    bool WaitForGPU() override;

    bool DownloadBuffer(const BufferPtr& buffer, size_t offset, size_t size, void* data) override;
    bool DownloadTexture(const TexturePtr& tex, void* data, int mipmap, int layer) override;

    NFE_INLINE bool IsDebugLayerEnabled()
    {
        return mDebugLayerEnabled;
    }
};

} // namespace Renderer
} // namespace NFE
