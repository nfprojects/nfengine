/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan implementation of renderer's device
 */

#include "PCH.hpp"
#include "Device.hpp"

// modules


namespace {

template<typename Type, typename Desc>
Type* GenericCreateResource(const Desc& desc)
{
    Type* resource = new (std::nothrow) Type;
    if (resource == nullptr)
        return nullptr;

    if (!resource->Init(desc))
    {
        delete resource;
        return nullptr;
    }

    return resource;
}

} // namespace

namespace NFE {
namespace Renderer {

std::unique_ptr<Device> gDevice;



Device::Device()
{
}

Device::~Device()
{
}

bool Device::Init()
{
    if (!mVkInstance.Init(false))
    {
        LOG_ERROR("Vulkan instance failed to initialize");
        return false;
    }

    const VkInstance& instance = mVkInstance.Get();

    const char* enabledExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    VkDeviceCreateInfo devInfo;
    memset(&devinfo, 0, sizeof(devInfo));
    devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devInfo.pNext = nullptr;
    devInfo.queueCreateInfoCount = 1;
    devInfo.

    LOG_INFO("Vulkan device initialized successfully");
    return true;
}

void* Device::GetHandle() const
{
    return nullptr;
}


IVertexLayout* Device::CreateVertexLayout(const VertexLayoutDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IBuffer* Device::CreateBuffer(const BufferDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

ITexture* Device::CreateTexture(const TextureDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IBackbuffer* Device::CreateBackbuffer(const BackbufferDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IRenderTarget* Device::CreateRenderTarget(const RenderTargetDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IPipelineState* Device::CreatePipelineState(const PipelineStateDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

ISampler* Device::CreateSampler(const SamplerDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IShader* Device::CreateShader(const ShaderDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IShaderProgram* Device::CreateShaderProgram(const ShaderProgramDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IResourceBindingSet* Device::CreateResourceBindingSet(const ResourceBindingSetDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IResourceBindingLayout* Device::CreateResourceBindingLayout(const ResourceBindingLayoutDesc& desc)
{
    UNUSED(desc);
    return nullptr;
}

IResourceBindingInstance* Device::CreateResourceBindingInstance(IResourceBindingSet* set)
{
    UNUSED(set);
    return nullptr;
}

ICommandBuffer* Device::CreateCommandBuffer()
{
    return nullptr;
}

bool Device::GetDeviceInfo(DeviceInfo& info)
{
    UNUSED(info);
    return false;
}

bool Device::Execute(ICommandList* commandList)
{
    UNUSED(commandList);
    return false;
}

bool Device::DownloadBuffer(IBuffer* buffer, size_t offset, size_t size, void* data)
{
    UNUSED(buffer);
    UNUSED(offset);
    UNUSED(size);
    UNUSED(data);
    return false;
}

bool Device::DownloadTexture(ITexture* tex, void* data, int mipmap, int layer)
{
    UNUSED(tex);
    UNUSED(data);
    UNUSED(mipmap);
    UNUSED(layer);
    return false;
}

IDevice* Init()
{
    if (gDevice == nullptr)
    {
        gDevice.reset(new (std::nothrow) Device);
        if (!gDevice->Init())
        {
            gDevice.reset();
            return nullptr;
        }
    }

    return gDevice.get();
}

void Release()
{
    gDevice.reset();
}

} // namespace Renderer
} // namespace NFE
