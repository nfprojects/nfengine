/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Declaration of Vulkan renderer's buffer.
 */

#pragma once

#include "../RendererCommon/Buffer.hpp"
#include "IResource.hpp"

#include "Defines.hpp"


namespace NFE {
namespace Renderer {

class Buffer : public IBuffer, public IResource
{
    friend class CommandRecorder;
    friend class ResourceBindingInstance;

    VkBuffer mBuffer;
    VkDeviceMemory mBufferMemory;
    VkDeviceSize mBufferSize;
    VkDeviceSize mStructureSize;
    ResourceAccessMode mMode;

    uint32 mVolatileBinding;
    uint32 mVolatileDataOffset;

public:
    Buffer();
    ~Buffer();
    bool Init(const BufferDesc& desc);

    void* Map(size_t size, size_t offset) override;
    void Unmap() override;

    const Internal::ResourceType GetType() const override
    {
        return Internal::ResourceType::Buffer;
    }
};

} // namespace Renderer
} // namespace NFE
