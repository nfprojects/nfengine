/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of renderer's Texture object
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "Texture.hpp"
#include "Device.hpp"

#include "Internal/Translations.hpp"


namespace NFE {
namespace Renderer {

Texture::Texture()
    : mType(TextureType::Unknown)
    , mWidth(1)
    , mHeight(1)
    , mDepth(1)
    , mFormat(VK_FORMAT_UNDEFINED)
    , mCurrentBuffer(0)
    , mFromSwapchain(false)
{
}

Texture::~Texture()
{
    // TODO this needs to go away
    mDevicePtr->WaitForGPU();

    for (auto& i : mImages)
    {
        if (i.view != VK_NULL_HANDLE)
            vkDestroyImageView(mDevicePtr->GetDevice(), i.view, nullptr);

        if (!mFromSwapchain)
        {
            if (i.memory != VK_NULL_HANDLE)
                vkFreeMemory(mDevicePtr->GetDevice(), i.memory, nullptr);
            if (i.image != VK_NULL_HANDLE)
                vkDestroyImage(mDevicePtr->GetDevice(), i.image, nullptr);
        }
    }
}

bool Texture::Init(Common::SharedPtr<Device>& device, const TextureDesc& desc)
{
    mDevicePtr = device;

    // TODO
    if (desc.type == TextureType::TextureCube)
    {
        NFE_LOG_ERROR("Cube textures unsupported");
        return false;
    }

    // TODO we might want to create empty textures without initial data
    if (desc.dataDesc == nullptr)
    {
        NFE_LOG_ERROR("No initial data provided for Image");
        return false;
    }

    mImages.Resize(1);

    mType = desc.type;
    if (desc.depthBufferFormat != DepthBufferFormat::Unknown)
        mFormat = TranslateDepthFormatToVkFormat(desc.depthBufferFormat);
    else
        mFormat = TranslateElementFormatToVkFormat(desc.format);

    VkResult result = VK_SUCCESS;

    // create buffer to store texture data
    VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
    VkBuffer stagingBuffer = VK_NULL_HANDLE;

    VkBufferCreateInfo bufInfo;
    VK_ZERO_MEMORY(bufInfo);
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    // TODO multiple dataDesc support
    switch (mType)
    {
    case TextureType::Texture1D:
        bufInfo.size = desc.dataDesc[0].lineSize;
        break;
    case TextureType::Texture2D:
        bufInfo.size = desc.dataDesc[0].sliceSize;
        break;
    case TextureType::Texture3D:
        bufInfo.size = desc.dataDesc[0].sliceSize * desc.width;
        break;
    default:
        NFE_LOG_ERROR("Unsupported or incorrect texture type.");
        return false;
    }

    bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    result = vkCreateBuffer(mDevicePtr->GetDevice(), &bufInfo, nullptr, &stagingBuffer);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to create staging buffer for data upload");

    VkMemoryRequirements stagingMemReqs;
    vkGetBufferMemoryRequirements(mDevicePtr->GetDevice(), stagingBuffer, &stagingMemReqs);

    VkMemoryAllocateInfo memInfo;
    VK_ZERO_MEMORY(memInfo);
    memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memInfo.allocationSize = stagingMemReqs.size;
    memInfo.memoryTypeIndex = mDevicePtr->GetMemoryTypeIndex(stagingMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    result = vkAllocateMemory(mDevicePtr->GetDevice(), &memInfo, nullptr, &stagingBufferMemory);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to allocate memory for staging buffer");

    void* bufferData = nullptr;
    result = vkMapMemory(mDevicePtr->GetDevice(), stagingBufferMemory, 0, memInfo.allocationSize, 0, &bufferData);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to map staging buffer's memory");
    memcpy(bufferData, desc.dataDesc[0].data, static_cast<size_t>(bufInfo.size));
    vkUnmapMemory(mDevicePtr->GetDevice(), stagingBufferMemory);
    result = vkBindBufferMemory(mDevicePtr->GetDevice(), stagingBuffer, stagingBufferMemory, 0);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to bind staging buffer to its memory");

    // Image creation
    VkImageCreateInfo imageInfo;
    VK_ZERO_MEMORY(imageInfo);
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    if (mType == TextureType::TextureCube)
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    switch (mType)
    {
    case TextureType::Texture1D:
        imageInfo.imageType = VK_IMAGE_TYPE_1D;
        break;
    case TextureType::Texture2D:
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        break;
    case TextureType::Texture3D:
        imageInfo.imageType = VK_IMAGE_TYPE_3D;
        break;
    case TextureType::TextureCube:
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        break;
    default:
        NFE_LOG_ERROR("Unsupported or incorrect texture type.");
        return false;
    }

    mWidth = desc.width;
    if (mType == TextureType::Texture2D || mType == TextureType::Texture3D || mType == TextureType::TextureCube)
        mHeight = desc.height;
    if (mType == TextureType::Texture3D)
        mDepth = desc.depth;

    imageInfo.format = mFormat;
    imageInfo.extent.width = mWidth;
    imageInfo.extent.height = mHeight;
    imageInfo.extent.depth = mDepth;
    imageInfo.mipLevels = desc.mipmaps;
    imageInfo.arrayLayers = desc.layers;
    imageInfo.samples = TranslateSamplesNumToVkSampleCount(desc.samplesNum);
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

    // TODO SHADER_WRITABLE
    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_DEPTH)
    {
        imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        mSubresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_RENDERTARGET)
    {
        imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        mSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_SHADER)
        imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

    imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

    result = vkCreateImage(mDevicePtr->GetDevice(), &imageInfo, nullptr, &mImages[0].image);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to create Image for texture");

    VkMemoryRequirements imageMemReqs;
    vkGetImageMemoryRequirements(mDevicePtr->GetDevice(), mImages[0].image, &imageMemReqs);

    VkMemoryAllocateInfo imageMemInfo;
    VK_ZERO_MEMORY(imageMemInfo);
    imageMemInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    imageMemInfo.allocationSize = imageMemReqs.size;
    imageMemInfo.memoryTypeIndex = mDevicePtr->GetMemoryTypeIndex(imageMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    result = vkAllocateMemory(mDevicePtr->GetDevice(), &imageMemInfo, nullptr, &mImages[0].memory);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to allocate memory for Image");

    result = vkBindImageMemory(mDevicePtr->GetDevice(), mImages[0].image, mImages[0].memory, 0);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to bind Image to its memory");

    // TODO right now copying is done on a general queue, but the devices support separate Transfer queue.
    //      Consider moving copy command buffers to transfer queue if device makes it possible.
    VkCommandBuffer copyCmdBuffer;
    VkCommandBufferAllocateInfo cmdInfo;
    VK_ZERO_MEMORY(cmdInfo);
    cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdInfo.commandPool = mDevicePtr->GetCommandPool(QueueType::General);
    cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdInfo.commandBufferCount = 1;
    result = vkAllocateCommandBuffers(mDevicePtr->GetDevice(), &cmdInfo, &copyCmdBuffer);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to allocate a command buffer");

    VkCommandBufferBeginInfo cmdBeginInfo;
    VK_ZERO_MEMORY(cmdBeginInfo);
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    result = vkBeginCommandBuffer(copyCmdBuffer, &cmdBeginInfo);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to begin command rendering for buffer copy operation");

    TransitionCurrentTexture(copyCmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // TODO Multiple dataDescs support
    VkBufferImageCopy imageCopyRegion;
    VK_ZERO_MEMORY(imageCopyRegion);
    imageCopyRegion.bufferOffset = 0;
    imageCopyRegion.imageExtent.width = mWidth;
    imageCopyRegion.imageExtent.height = mHeight;
    imageCopyRegion.imageExtent.depth = mDepth;
    imageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCopyRegion.imageSubresource.layerCount = desc.layers;
    imageCopyRegion.imageSubresource.mipLevel = 0;
    vkCmdCopyBufferToImage(copyCmdBuffer, stagingBuffer, mImages[0].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);

    result = vkEndCommandBuffer(copyCmdBuffer);
    VK_RETURN_FALSE_IF_FAILED(result, "Failure during copy command buffer recording");

    VkSubmitInfo submitInfo;
    VK_ZERO_MEMORY(submitInfo);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &copyCmdBuffer;
    // FIXME vkQueueSubmit(mDevicePtr->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE);

    mDevicePtr->WaitForGPU();

    vkFreeCommandBuffers(mDevicePtr->GetDevice(), mDevicePtr->GetCommandPool(QueueType::General), 1, &copyCmdBuffer);
    vkFreeMemory(mDevicePtr->GetDevice(), stagingBufferMemory, nullptr);
    vkDestroyBuffer(mDevicePtr->GetDevice(), stagingBuffer, nullptr);

    VkImageViewCreateInfo ivInfo;
    VK_ZERO_MEMORY(ivInfo);
    ivInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivInfo.image = mImages[0].image;

    switch (mType)
    {
    case TextureType::Texture1D:
        ivInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
        break;
    case TextureType::Texture2D:
        ivInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        break;
    case TextureType::Texture3D:
        ivInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        break;
    case TextureType::TextureCube:
        ivInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        break;
    default:
        NFE_LOG_ERROR("Unsupported or incorrect texture type.");
        return false;
    }

    ivInfo.format = imageInfo.format;
    ivInfo.components = {
        // order of variables in VkComponentMapping is r, g, b, a
        VK_COMPONENT_SWIZZLE_R,
        VK_COMPONENT_SWIZZLE_G,
        VK_COMPONENT_SWIZZLE_B,
        VK_COMPONENT_SWIZZLE_A,
    };
    ivInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivInfo.subresourceRange.baseMipLevel = 0;
    ivInfo.subresourceRange.levelCount = 1;
    ivInfo.subresourceRange.layerCount = 1;
    result = vkCreateImageView(mDevicePtr->GetDevice(), &ivInfo, nullptr, &mImages[0].view);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to generate Image View from created Texure's image");

    NFE_LOG_INFO("Texture initialized successfully");
    return true;
}

void Texture::TransitionCurrentTexture(VkCommandBuffer cmdBuffer, VkImageLayout newLayout)
{
    if (newLayout == mImages[mCurrentBuffer].layout)
        return; // nothing to do

    VkImageMemoryBarrier imageBarrier{};
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier.image = mImages[mCurrentBuffer].image;
    imageBarrier.oldLayout = mImages[mCurrentBuffer].layout;
    imageBarrier.newLayout = newLayout;
    imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrier.subresourceRange.baseMipLevel = 0;
    imageBarrier.subresourceRange.levelCount = 1;
    imageBarrier.subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &imageBarrier);

    mImages[mCurrentBuffer].layout = newLayout;
}

} // namespace Renderer
} // namespace NFE
