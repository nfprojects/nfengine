/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of Vulkan vertex layout.
 */

#pragma once

#include "../RendererInterface/VertexLayout.hpp"
#include "Defines.hpp"
#include "PipelineState.hpp"
#include "nfCommon/Containers/DynArray.hpp"

namespace NFE {
namespace Renderer {

class VertexLayout : public IVertexLayout
{
    friend class PipelineState;

    Common::DynArray<VkVertexInputBindingDescription> mBindings;
    Common::DynArray<VkVertexInputAttributeDescription> mAttributes;

public:
    VertexLayout();
    bool Init(const VertexLayoutDesc& desc);
};

} // namespace Renderer
} // namespace NFE
