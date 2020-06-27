/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
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
    BufferMode mMode;
    BufferType mType;

    uint32 mVolatileBinding;
    uint32 mVolatileDataOffset;

public:
    Buffer();
    ~Buffer();
    bool Init(const BufferDesc& desc);

    const ShaderResourceType GetType() const override
    {
        // type doesn't exactly match, but it's used only for ResourceBinding purposes
        return ShaderResourceType::CBuffer;
    }
};

} // namespace Renderer
} // namespace NFE
