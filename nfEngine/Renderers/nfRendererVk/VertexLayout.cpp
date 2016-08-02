/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan implementation of renderer's vertex layout
 */

#include "PCH.hpp"
#include "VertexLayout.hpp"
#include "Translations.hpp"
#include "Shader.hpp"

namespace NFE {
namespace Renderer {

VertexLayout::VertexLayout()
{
}

bool VertexLayout::Init(const VertexLayoutDesc& desc)
{
    VkVertexInputBindingDescription vibDesc;
    VK_ZERO_MEMORY(vibDesc);

    VkVertexInputAttributeDescription viaDesc;
    VK_ZERO_MEMORY(viaDesc);

    // gather how much VBs we have to bind
    std::list<std::tuple<int, int>> vbList;
    for (int i = 0; i < desc.numElements; ++i)
    {
        vbList.push_back({ i, desc.elements[i].vertexBufferId });
    }

    // sort according to VB ID
    vbList.sort([](const std::tuple<int, int>& first, const std::tuple<int, int>& second)
    {
        return (std::get<1>(first) < std::get<1>(second));
    });

    // remove spare VB IDs
    vbList.unique([](const std::tuple<int, int>& first, const std::tuple<int, int>& second)
    {
        return (std::get<1>(first) == std::get<1>(second));
    });

    // create bindings
    for (auto& vb : vbList)
    {
        vibDesc.binding = std::get<1>(vb);
        vibDesc.stride = TranslateElementFormatToByteSize(desc.elements[std::get<0>(vb)].format)
                       * desc.elements[std::get<0>(vb)].size;
        vibDesc.inputRate = desc.elements[std::get<0>(vb)].perInstance ? VK_VERTEX_INPUT_RATE_INSTANCE
                                                                       : VK_VERTEX_INPUT_RATE_VERTEX;
        mBindings.push_back(vibDesc);
    }
    
    // create attributes
    for (int i = 0; i < desc.numElements; ++i)
    {
        viaDesc.binding = desc.elements[i].vertexBufferId;
        viaDesc.location = i;
        viaDesc.format = TranslateElementFormatToVkFormat(desc.elements[i].format, desc.elements[i].size);
        viaDesc.offset = desc.elements[i].offset;
        mAttributes.push_back(viaDesc);
    }

    return true;
}

} // namespace Renderer
} // namespace NFE
