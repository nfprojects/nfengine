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
    friend class CommandRecorder;
    friend class ResourceBindingInstance;

    Common::SharedPtr<Device> mDevicePtr;

    VkBuffer mBuffer;
    VkDeviceMemory mBufferMemory;
    VkDeviceSize mBufferSize;
    BufferMode mMode;

public:
    Buffer();
    ~Buffer();
    bool Init(Common::SharedPtr<Device> device, const BufferDesc& desc);
};

} // namespace Renderer
} // namespace NFE
