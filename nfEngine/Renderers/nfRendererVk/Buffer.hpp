/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of Vulkan renderer's buffer.
 */

#pragma once

#include "../RendererInterface/Buffer.hpp"

#include "Defines.hpp"


namespace NFE {
namespace Renderer {

class Buffer : public IBuffer
{
    friend class CommandBuffer;
    friend class ResourceBindingInstance;

    VkBuffer mBuffer;
    VkDeviceMemory mBufferMemory;
    VkDeviceSize mBufferSize;
    BufferMode mMode;

public:
    Buffer();
    ~Buffer();
    bool Init(const BufferDesc& desc);
};

} // namespace Renderer
} // namespace NFE
