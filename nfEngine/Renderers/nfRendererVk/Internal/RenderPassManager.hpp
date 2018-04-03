/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of Vulkan's Render Pass Manager.
 */

#pragma once

#include "Defines.hpp"
#include "RenderPassDesc.hpp"

#include "nfCommon/Containers/HashMap.hpp"


namespace NFE {
namespace Renderer {

/**
 * A class managing Vulkan's Render Passes.
 *
 * It's main goal is to provide a user with VkRenderPass objects, reducing
 * their amount to minimum. Render Passes only need formats to be created -
 * if any part of nfRendererVk asks for a specific Render Pass (which is
 * one created for specific formats), this class will reuse any already
 * created Render Pass, or will create a new one.
 */
class RenderPassManager
{
    VkDevice mDeviceRef;

    using RenderPassMap = Common::HashMap<RenderPassDesc, VkRenderPass>;
    RenderPassMap mRenderPasses;

    VkRenderPass ConstructRenderPass(const RenderPassDesc& desc);

public:
    RenderPassManager(VkDevice device);
    ~RenderPassManager();

    VkRenderPass GetRenderPass(const RenderPassDesc& desc);
};

} // namespace Renderer
} // namespace NFE
