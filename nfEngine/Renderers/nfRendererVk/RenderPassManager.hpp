/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of Vulkan's Render Pass Manager.
 */

#pragma once

#include "PCH.hpp"
#include "Defines.hpp"
#include "RenderPassDesc.hpp"


namespace NFE {
namespace Renderer {

class RenderPassManager
{
    VkDevice mDeviceRef;

    typedef std::unordered_map<RenderPassDesc, VkRenderPass> RenderPassMap;

    RenderPassMap mRenderPasses;

    VkRenderPass ConstructRenderPass(const RenderPassDesc& desc);

public:
    RenderPassManager(VkDevice device);
    ~RenderPassManager();

    VkRenderPass GetRenderPass(const RenderPassDesc& desc);
};

} // namespace Renderer
} // namespace NFE
