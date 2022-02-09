#pragma once

#include "API/IResource.hpp"
#include "Engine/Common/Containers/Set.hpp"
#include "Engine/Common/Containers/HashMap.hpp"
#include "Types.hpp"


namespace NFE {
namespace Renderer {


class LayoutTracker
{
    struct ResourceData
    {
        VkImage image;
        VkImageSubresourceRange subresRange;
        VkImageLayout layout;

        ResourceData()
            : image(VK_NULL_HANDLE)
            , subresRange{0, 0, 0, 0, 0}
            , layout(VK_IMAGE_LAYOUT_UNDEFINED)
        {
        }

        ResourceData(VkImage image, VkImageSubresourceRange range)
            : image(image)
            , subresRange(range)
            , layout(VK_IMAGE_LAYOUT_UNDEFINED)
        {
        }
    };

    uint32 mNewIDCounter;
    Common::HashMap<Internal::ResourceID, ResourceData> mResources;
    Common::Mutex mMutex;

public:
    LayoutTracker();
    ~LayoutTracker();

    Internal::ResourceID Register(VkImage image, VkImageSubresourceRange range);
    void Unregister(Internal::ResourceID resourceId);

    void EnsureLayout(VkCommandBuffer cmdBuffer, Internal::ResourceID resourceId, VkImageLayout layout);
};


} // namespace Renderer
} // namespace NFE
