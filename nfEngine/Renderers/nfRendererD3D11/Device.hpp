/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 11 render's device.
 */

#pragma once

#include "../RendererInterface/Device.hpp"
#include "Common.hpp"
#include "nfCommon/System/Mutex.hpp"


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

    Common::DynArray<D3DPtr<IDXGIAdapter>> mAdapters;
    int mAdapterInUse;

    D3D_FEATURE_LEVEL mFeatureLevel;
    bool mDebugLayerEnabled;

    Common::Mutex mCommandListsMutex;
    Common::DynArray<ID3D11CommandList*> mCommandLists;

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
    ResourceBindingInstancePtr CreateResourceBindingInstance(const ResourceBindingSetPtr& set) override;

    CommandRecorderPtr CreateCommandRecorder() override;
    bool Execute(CommandListID id) override;
    bool FinishFrame() override;
    bool WaitForGPU() override;

    bool DownloadBuffer(const BufferPtr& buffer, size_t offset, size_t size, void* data) override;
    bool DownloadTexture(const TexturePtr& tex, void* data, uint32 mipmap, uint32 layer) override;

    NFE_INLINE bool IsDebugLayerEnabled()
    {
        return mDebugLayerEnabled;
    }

    // registers a recorded command list for current frame
    CommandListID RegisterCommandList(ID3D11CommandList* commandList);
};

} // namespace Renderer
} // namespace NFE
