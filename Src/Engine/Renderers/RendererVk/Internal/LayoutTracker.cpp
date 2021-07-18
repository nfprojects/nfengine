#include "PCH.hpp"
#include "LayoutTracker.hpp"
#include "API/Backbuffer.hpp"
#include "API/Texture.hpp"
#include "Types.hpp"


namespace NFE {
namespace Renderer {


LayoutTracker::LayoutTracker()
    : mNewIDCounter(0)
    , mResources()
    , mMutex()
{
}

LayoutTracker::~LayoutTracker()
{
}

Internal::ResourceID LayoutTracker::Register(VkImage image, VkImageSubresourceRange range)
{
    uint32 id = mNewIDCounter++;
    ResourceData data(image, range);
    mResources.Insert(id, data);
    return id;
}

void LayoutTracker::Unregister(Internal::ResourceID resourceId)
{
    NFE_ASSERT(mResources.Exists(resourceId), "Resource ID invalid");
    mResources.Erase(resourceId);
}

void LayoutTracker::EnsureLayout(VkCommandBuffer cmdBuffer, Internal::ResourceID resourceId, VkImageLayout layout)
{
    ResourceData& data = mResources.Find(resourceId)->second;

    if (data.layout == layout)
        return;

    VkImageMemoryBarrier imageBarrier;
    VK_ZERO_MEMORY(imageBarrier);
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.srcAccessMask = 0;
    imageBarrier.dstAccessMask = 0;
    imageBarrier.oldLayout = data.layout;
    imageBarrier.newLayout = layout;
    imageBarrier.image = data.image;
    imageBarrier.subresourceRange = data.subresRange;

    // assume all barriers are full blocking, like in D3D12 renderer
    vkCmdPipelineBarrier(cmdBuffer,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                         0, nullptr, 0, nullptr, 1, &imageBarrier);

    data.layout = layout;
}


} // namespace Renderer
} // namespace NFE
