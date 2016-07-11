/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan Device
 */

#pragma once

#include "Defines.hpp"

#include "../RendererInterface/Device.hpp"

#include "CommandBuffer.hpp"
#include "Instance.hpp"


namespace NFE {
namespace Renderer {

class Device : public IDevice
{
private:
    Instance mVkInstance;
    VkDevice mVkDevice;
    VkPhysicalDevice mVkPhysicalDevice;

    VkPhysicalDevice SelectPhysicalDevice(const std::vector<VkPhysicalDevice>& devices);

public:
    Device();
    ~Device();

    bool Init();

    // overrides
    void* GetHandle() const;
    bool GetDeviceInfo(DeviceInfo& info);

    IVertexLayout* CreateVertexLayout(const VertexLayoutDesc& desc);
    IBuffer* CreateBuffer(const BufferDesc& desc);
    ITexture* CreateTexture(const TextureDesc& desc);
    IBackbuffer* CreateBackbuffer(const BackbufferDesc& desc);
    IRenderTarget* CreateRenderTarget(const RenderTargetDesc& desc);
    IPipelineState* CreatePipelineState(const PipelineStateDesc& desc);
    ISampler* CreateSampler(const SamplerDesc& desc);
    IShader* CreateShader(const ShaderDesc& desc);
    IShaderProgram* CreateShaderProgram(const ShaderProgramDesc& desc);
    IResourceBindingSet* CreateResourceBindingSet(const ResourceBindingSetDesc& desc);
    IResourceBindingLayout* CreateResourceBindingLayout(const ResourceBindingLayoutDesc& desc);
    IResourceBindingInstance* CreateResourceBindingInstance(IResourceBindingSet* set);

    ICommandBuffer* CreateCommandBuffer();
    bool Execute(ICommandList* commandList);
    bool DownloadBuffer(IBuffer* buffer, size_t offset, size_t size, void* data);
    bool DownloadTexture(ITexture* tex, void* data, int mipmap = 0, int layer = 0);
};

extern std::unique_ptr<Device> gDevice;

extern "C" RENDERER_API IDevice* Init();
extern "C" RENDERER_API void Release();

} // namespace Renderer
} // namepsace NFE
